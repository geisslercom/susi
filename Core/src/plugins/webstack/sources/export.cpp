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

#include "HttpServerComponent.h"
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
	std::string address{""};
	size_t threads{4};
	size_t backlog{16};

	if(config["address"].isString()){
		address = static_cast<std::string>(config["address"]);
	}
	std::string assetRoot{""};
	if(config["assets"].isString()){
		assetRoot = static_cast<std::string>(config["assets"]);
	}
	std::string upload{""};
	if(config["upload"].isString()){
		upload = static_cast<std::string>(config["upload"]);
	}

	if(config["threads"].isInteger()){
		threads =  static_cast<long>(config["threads"]);
	}
	if(config["backlog"].isInteger()){
		backlog =  static_cast<long>(config["backlog"]);
	}
    return std::make_shared<Susi::HttpServerComponent>(mgr,address,assetRoot,upload,threads,backlog);
}
std::string LIBRARY_API getName(){
    return "webstack";
}
std::vector<std::string> LIBRARY_API getDependencies(){
    return {};
}

