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

#include "SyscallComponent.h"
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
	size_t threads = 4;
	size_t queuelen = 16;
	Susi::Util::Any::Object commands = config["commands"];
	Susi::Util::Any threadsVal = config["threads"];
	if(!threadsVal.isNull()){
		threads = (size_t)static_cast<long>(threadsVal);
	}
	Susi::Util::Any queuelenVal = config["queuelen"];
	if(!queuelenVal.isNull()){
		queuelen = (size_t)static_cast<long>(queuelenVal);
	}
    return std::make_shared<Susi::Syscall::SyscallComponent>(mgr,commands,threads,queuelen);
}
std::string LIBRARY_API getName(){
    return "syscalls";
}
std::vector<std::string> LIBRARY_API getDependencies(){
    return {"eventsystem"};
}

