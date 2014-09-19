#include "sessions/SessionManagerComponent.h"


void Susi::Sessions::SessionManagerComponent::handleCheckSessions(Susi::Events::SharedEventPtr event) {
	checkSessions();
}

void Susi::Sessions::SessionManagerComponent::handleGetAttribute(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		std::string key = event->payload["key"];

		event->payload["value"] = getSessionAttribute(sessionID, key);
	}catch(const std::exception & e){
		std::string msg = "Error in handleSessionGetAttribute(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}
void Susi::Sessions::SessionManagerComponent::handleSetAttribute(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		std::string key = event->payload["key"];
		Susi::Util::Any value = event->payload["value"];

		event->payload["success"] = setSessionAttribute(sessionID, key, value);
	}catch(const std::exception & e){
		std::string msg = "Error in handleSessionSetAttribute(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}
void Susi::Sessions::SessionManagerComponent::handlePushAttribute(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		std::string key = event->payload["key"];
		Susi::Util::Any value = event->payload["value"];

		event->payload["success"] = pushSessionAttribute(sessionID, key, value);
	}catch(const std::exception & e){
		std::string msg = "Error in handleSessionPushAttribute(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}
void Susi::Sessions::SessionManagerComponent::handleRemoveAttribute(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		std::string key = event->payload["key"];

		event->payload["success"] = removeSessionAttribute(sessionID, key);
	}catch(const std::exception & e){
		std::string msg = "Error in handleSessionRemoveAttribute(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}
void Susi::Sessions::SessionManagerComponent::handleUpdate(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		std::chrono::seconds secs;

		int seconds = event->payload["seconds"];

		if(seconds > 0) {
			secs = std::chrono::seconds(seconds);
			updateSession(sessionID, std::chrono::duration_cast<std::chrono::milliseconds>(secs));
		}
		else {
			updateSession(sessionID);
		}

		event->payload["success"] = true;
	}catch(const std::exception & e){
		std::string msg = "Error in handleUpdateSession(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}
void Susi::Sessions::SessionManagerComponent::handleCheck(Susi::Events::EventPtr event) {
	try{
		std::string sessionID = event->payload["id"];
		event->payload["success"] = checkSession(sessionID);
	}catch(const std::exception & e){
		std::string msg = "Error in handleCheckSession(): ";
		msg += e.what();
		throw std::runtime_error(msg);
	}
}