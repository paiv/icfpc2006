#include "unity.cpp"

#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <fcntl.h>
#include <zmq.hpp>
#include <signal.h>
#include <sys/stat.h>

#include "disassembler.cpp"
#include "debugger.cpp"
#include "commands.cpp"


int main(int argc, char* argv[]) {

  if (argc < 2) {
    cerr << "usage: udb <file.um>" << endl;
    return 1;
  }

  // parent process -> zmq -> child process [command handler]
  // child [command handler] -> vm worker thread

  // stdin -> parent process
  // parent -> pty -> child [stdin]
  // stdout <- parent
  // stdout <- child

  int fdparent = posix_openpt(O_RDWR);
  grantpt(fdparent);
  unlockpt(fdparent);

  int fdchild = open(ptsname(fdparent), O_RDWR);

  int fd = fork();

  if (fd != 0)
  {
    close(fdchild);

    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REQ);
    socket.connect("tcp://127.0.0.1:7199");

    CommandDispatcher dispatcher(fdparent, &socket);
    Command lastCommand("");

    while (true)
    {
      usleep(42000);
      char* line = readline("> ");
      if (!line)
        break;
      add_history(line);

      Command command(line);
      free(line);

      if (command.name == "")
        command = lastCommand;

      if (!dispatcher.process(command))
        break;

      lastCommand = command;
    }

    int state;
    if (!waitpid(fd, &state, WNOHANG))
      kill(fd, SIGQUIT);
  }
  else
  {
    close(STDIN_FILENO);
    dup2(fdchild, STDIN_FILENO);
    close(fdchild);

    zmq::context_t context;
    zmq::socket_t socket(context, ZMQ_REP);
    socket.bind("tcp://*:7199");

    ImageLoader loader;
    auto program = loader.read(argv[1]);

    CommandHandler handler(&context, &socket, fdparent, program);
    handler.run();
  }

  cout << endl;
  return 0;
}
