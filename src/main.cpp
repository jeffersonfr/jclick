#include "mainframe.h"
#include "config.h"
#include "jlocalipcclient.h"
#include "jremoteipcclient.h"
#include "jipcexception.h"

void usage()
{
	std::cout << "..:: " << __LOCAL_MODULE_NAME__ << " ::.." << std::endl;
	std::cout << "" << std::endl;
	std::cout << "  --help: exibe este menu de ajuda" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "  --timeout: input method's timeout(ms)" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "  --input: envia comandos para um sistema remoto (/etc/" << __LOCAL_MODULE_NAME__ << "/system.conf:camera.input)" << std::endl;
	std::cout << "" << std::endl;
	std::cout << "      local:<id> <method> <param=value> ... <param=value>" << std::endl;
	std::cout << "      remote:<ip:port> <method> <param=value> ... <param=value>" << std::endl;
	std::cout << "" << std::endl;

	exit(0);
}

void version()
{
	std::cout << "verson 1.0.0a" << std::endl;

	exit(0);
}

void cleanup_resources()
{
	// INFO:: remove local socket file
	camera_input_t t = __C->GetCameraInput();

	if (t.is_local == true) {
		std::string command = "rm -rf /tmp/" + t.id + ".socket";

		if (system(command.c_str()) != 0) {
			std::cout << "[" << t.id << ".socket] removed" << std::endl;
		}
	}
	
	// INFO:: remove temporaty directory
	std::string temporary = __C->GetTempPath();
	
	std::string command = "rm -rf " + temporary;

	if (system(command.c_str()) != 0) {
		std::cout << "[" << temporary << "] removed" << std::endl;
	}

	exit(0);
}

void signal_handler(int)
{
	cleanup_resources();
}

int main(int argc, char **argv)
{
	struct sigaction sa_old;
	struct sigaction sa_new;

	memset(&sa_old, 0, sizeof(sa_old));
	memset(&sa_new, 0, sizeof(sa_new));

	// sigemptyset(&sa_new.sa_mask);
	sigfillset(&sa_new.sa_mask);
	sa_new.sa_handler = signal_handler;
	sa_new.sa_flags = SA_RESTART | SA_RESETHAND | SA_NODEFER;

	if (argc > 1) {
		std::string input;
		int i, timeout = -1;

		for (i=1; i<argc; i++) {
			if (strcmp(argv[i], "--help") == 0) {
				usage();
			} else if (strcmp(argv[i], "--version") == 0) {
				version();
			} else if (strcmp(argv[i], "--input") == 0) {
				if (i < (argc-1)) {
					input = argv[++i];
				} else {
					JDEBUG(JWARN, "parser failed [--input]\n");

					return -1;
				}
			} else if (strcmp(argv[i], "--timeout") == 0) {
				if (i < (argc-1)) {
					timeout = atoi(argv[++i]);
				} else {
					JDEBUG(JWARN, "parser failed [--timeout]\n");

					return -1;
				}
			} else {
				break;
			}
		}
			
		jcommon::StringTokenizer tokens(input, ":");

		jipc::IPCClient *client = NULL;

		if (tokens.GetToken(0) == "local") {
			if (tokens.GetSize() != 2) {
				JDEBUG(JWARN, "parser failed [--input %s]\n", input.c_str());

				return -1;
			}

			client = new jipc::LocalIPCClient(tokens.GetToken(1));
		} else {
			if (tokens.GetSize() != 3) {
				JDEBUG(JWARN, "parser failed [--input %s]\n", input.c_str());

				return -1;
			}

			client = new jipc::RemoteIPCClient(tokens.GetToken(1), atoi(tokens.GetToken(2).c_str()));
		}

		jipc::Method method(argv[i]);

		for (i=i+1; i<argc; i++) {
			std::string param = std::string(argv[i]);
			std::string::size_type r = param.find("=");

			if (r != std::string::npos) {
				method.SetTextParam(param.substr(0, r), param.substr(r+1));
			}
		}

		jipc::Response *response = NULL;

		std::cout << "Client request [" << method.what() << "]" << std::endl;

		try {
			client->SetRequestTimeout(timeout);
			client->CallMethod(&method, &response);
		
			std::cout << "Server response [" << response->what() << "]" << std::endl;
		} catch (jipc::IPCException &e) {
			JDEBUG(JERROR, "%s\n", e.what().c_str());
		}

		delete client;

		return 0;
	}

	// INFO:: just remove files if we had create them
	sigaction(SIGABRT, &sa_new, &sa_old);
	sigaction(SIGINT, &sa_new, &sa_old);
	sigaction(SIGSEGV, &sa_new, &sa_old);

	atexit(cleanup_resources);

	try {
		MainFrame window;

		window.Initialize();
	} catch (jcommon::RuntimeException &e) {
		e.PrintStackTrace();
	}

	jgui::GFXHandler::GetInstance()->Release();

	return 0;
}
