/*
 * Copyright (c) 2014, webvariants GmbH, http://www.webvariants.de
 *
 * This file is released under the terms of the MIT license. You can find the
 * complete text in the attached LICENSE file or online at:
 *
 * http://www.opensource.org/licenses/mit-license.php
 * 
 * @author: Tino Rusch (tino.rusch@webvariants.de)
 */

#ifndef __SUSISERVERCOMPONENTMANAGER__
#define __SUSISERVERCOMPONENTMANAGER__

#include "world/PluginLoadingComponentManager.h"
#include "util/Any.h"
 
#include "events/EventManagerComponent.h"
#include "logger/Logger.h"

namespace Susi {
namespace System {

class SusiServerComponentManager : public PluginLoadingComponentManager {
public:
	SusiServerComponentManager(Susi::Util::Any::Object config);
};

}
}

#endif // __SUSISERVERCOMPONENTMANAGER__
