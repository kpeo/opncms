#include <cppcms/service.h>
#include <cppcms/applications_pool.h>
#include <opncms/site.h>
#include <boost/thread/thread.hpp> 
#include <boost/thread/tss.hpp>
#include "version.h"


/* WARN: restart on SIGHUP is not working yet. 
 * Please use SIGTERM and start application again.
 */

static int caught_signal = 0;
cppcms::service* service = NULL;

#ifdef _OPNCMS_WIN_NATIVE_
BOOL WINAPI signal_handler(DWORD ctrl_type)
{
	switch (ctrl_type)
	{
		case CTRL_SHUTDOWN_EVENT:
			if(service)
				service->shutdown();
			return TRUE;
		default:
			return FALSE;
	}
}
#else
void signal_handler(int signal_number)
{
	std::cerr << "Received " << signal_number << " signal" << std::endl;
	caught_signal = signal_number;
	if(service)
		service->shutdown();
}
#endif

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		std::cout << "opnCMS v" << opncms_VERSION_MAJOR << "." << opncms_VERSION_MINOR << "." << opncms_VERSION_PATCH << " rev." << opncms_REVISION << std::endl;
		std::cout << "Usage: " << argv[0] << " [-c config]" << std::endl;
		return 1;
	}

#ifdef _OPNCMS_WIN_NATIVE_
	SetConsoleCtrlHandler(signal_handler, TRUE);
#else
	if (signal(SIGHUP, signal_handler) == SIG_ERR)
		std::cout << "Can't catch SIGHUP signal" << std::endl;
#endif

	while(1)
	{
		try {
			cppcms::service srv(argc,argv);
			service = &srv;
			srv.applications_pool().mount(cppcms::applications_factory<apps::site>(argc,argv) );
			srv.run();

			if(caught_signal)
			{
				switch (caught_signal)
				{
					case SIGALRM:
					case SIGHUP:
					case SIGINT:
					case SIGTERM:
					default:{
						//terminate
						//caught_signal = 0;
						std::cout << "Application stopped" << std::endl;
						return 0;
					}
				}
			}
		}
		catch(std::exception const &e) {
			BOOSTER_ERROR("opncms") << e.what();
			std::cerr << e.what() << std::endl;
			return 1;
		}
		sleep(1);
	}
	return 0;
}
