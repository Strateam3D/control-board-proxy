#include <iostream>

#include "eq-common/Dimensions.hpp"
#include "app/ControlBoardApp.hpp"

#include <boost/program_options.hpp>

using namespace strateam::equipment;
using namespace std::literals::chrono_literals;

namespace po = boost::program_options;

static void signal_handler(int signo){
    switch (signo){
    case SIGINT:
    case SIGTERM:
        std::cout << "Caught SIGTERM. alarm(20) was set, pls wait." << std::endl;
        alarm( 20 );
        strateam::control_board::ControlBoardApp::getInstance().shutdown();
        break;
    default:
        std::cout << "unknow signal" << std::endl;
    }
}

void install_signal_handlers(){
   if ( std::signal( SIGINT, signal_handler ) == SIG_ERR ){
      std::cout << "Couldn't install signal handler for SIGINT" << std::endl;
      exit( -1 );
   }

   if ( std::signal( SIGTERM, signal_handler ) == SIG_ERR ){
      std::cout << "Couldn't install signal handler for SIGTERM" << std::endl;
      exit( -1 );
   }
}


int main( int argc, char** argv ){
    po::options_description desc("Options");
    desc.add_options()
        ("help", "./control_board -c /path/to/cfg")
        ("c", po::value<std::string>(), "path to config")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << "./control_board -c /path/to/cfg" << "\n";
        return 0;
    }

    std::string cfg;

    if (vm.count("c")) {
        cfg = vm["c"].as<std::string>();
        std::cout << "path " << cfg << "\n";
    } else {
        std::cout << "Path not provided.\n";
        return -1;
    }

    install_signal_handlers();
    
    try{
        auto & app = strateam::control_board::ControlBoardApp::getInstance();
        app.init( cfg );
        app.run();
        return 0;
    }catch( std::exception const& ex ){
        std::cout << ex.what() << std::endl;
        return -1;
    }

    return 0;
}