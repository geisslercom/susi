/*
 * Copyright (c) 2015, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 *
 * @author: Tino Rusch (tino.rusch@webvariants.de)
 */

#include "SessionManagerComponent.h"
#include "logger/easylogging++.h"
SHARE_EASYLOGGINGPP(el::Helpers::storage());

#if defined(_WIN32)
#define LIBRARY_API __declspec(dllexport)
#else
#define LIBRARY_API
#endif

extern "C" {
    std::shared_ptr<Susi::System::Component> LIBRARY_API createComponent(Susi::System::ComponentManager *mgr, Susi::Util::Any & config);
    std::string LIBRARY_API getName();
    std::vector<std::string> LIBRARY_API getDependencies();
}

std::shared_ptr<Susi::System::Component> LIBRARY_API createComponent(Susi::System::ComponentManager * mgr, Susi::Util::Any & config){
	std::chrono::milliseconds lifetime{10000};		
	if(config["lifetime"].isInteger()){
		lifetime =  std::chrono::milliseconds{static_cast<int>(config["lifetime"])};
	}
    return std::make_shared<Susi::Sessions::SessionManagerComponent>(mgr,lifetime);
}
std::string LIBRARY_API getName(){
    return "sessionmanager";
}
std::vector<std::string> LIBRARY_API getDependencies(){
    return {"eventsystem"};
}

