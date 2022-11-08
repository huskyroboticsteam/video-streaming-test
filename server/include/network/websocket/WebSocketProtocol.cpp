#include "WebSocketProtocol.h"
#include <cstdio>
namespace net {
namespace websocket {

static const std::string TYPE_KEY = "type";

WebSocketProtocol::WebSocketProtocol(const std::string& protocolPath)
	: protocolPath(protocolPath), handlerMap(), validatorMap() {}

bool WebSocketProtocol::addMessageHandler(const std::string& messageType,
										  const msghandler_t& callback) {
	return this->addMessageHandler(messageType, callback, [](const json&) { return true; });
}

bool WebSocketProtocol::addMessageHandler(const std::string& messageType,
										  const msghandler_t& callback,
										  const validator_t& validator) {
	if (!hasMessageHandler(messageType)) {
		handlerMap[messageType] = callback;
		validatorMap[messageType] = validator;
		return true;
	}
	return false;
}

bool WebSocketProtocol::hasMessageHandler(const std::string& messageType) const {
	return handlerMap.find(messageType) != handlerMap.end();
}

bool WebSocketProtocol::removeMessageHandler(const std::string& messageType) {
	if (handlerMap.erase(messageType) != 0) {
		validatorMap.erase(messageType);
		return true;
	}
	return false;
}

void WebSocketProtocol::addConnectionHandler(const connhandler_t& handler) {
	connectionHandlers.push_back(handler);
}

void WebSocketProtocol::addDisconnectionHandler(const connhandler_t& handler) {
	disconnectionHandlers.push_back(handler);
}

void WebSocketProtocol::clientConnected() {
	for (const auto& f : connectionHandlers) {
		f();
	}
}

void WebSocketProtocol::clientDisconnected() {
	for (const auto& f : disconnectionHandlers) {
		f();
	}
}

void WebSocketProtocol::processMessage(const json& obj) const {
	if (obj.contains(TYPE_KEY)) {
		std::string messageType = obj[TYPE_KEY];
		auto validatorEntry = validatorMap.find(messageType);
		if (validatorEntry != validatorMap.end()) {
			if (validatorEntry->second(obj)) {
				if (false) { // protocolPath == Constants::MC_PROTOCOL_NAME
					printf("MC->R: %s\n", obj.dump().c_str());
				}
				handlerMap.at(messageType)(obj);
			} else {
				printf("Endpoint=%s : Invalid message received of type=%s: %s\n",
					protocolPath.c_str(), messageType.c_str(), obj.dump().c_str());
			}
		} else {
			printf("Endpoint=%s : Unrecognized message type: %s\n",
				protocolPath.c_str(), messageType.c_str());
		}
	} else {
		printf("Endpoint=%s : Malformed message without type key: %s\n",
			protocolPath.c_str(), obj.dump().c_str());
	}
}

std::string WebSocketProtocol::getProtocolPath() const {
	return protocolPath;
}

} // namespace websocket
} // namespace net
