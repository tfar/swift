/*
 * Copyright (c) 2013-2016 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#pragma clang diagnostic ignored "-Wunused-private-field"

#include <Swiften/Serializer/PayloadSerializers/PubSubEventItemSerializer.h>
#include <Swiften/Serializer/XML/XMLElement.h>
#include <memory>

#include <Swiften/Serializer/PayloadSerializerCollection.h>
#include <Swiften/Base/foreach.h>
#include <Swiften/Serializer/XML/XMLRawTextNode.h>

using namespace Swift;

PubSubEventItemSerializer::PubSubEventItemSerializer(PayloadSerializerCollection* serializers) : serializers(serializers) {
}

PubSubEventItemSerializer::~PubSubEventItemSerializer() {
}

std::string PubSubEventItemSerializer::serializePayload(std::shared_ptr<PubSubEventItem> payload) const {
    if (!payload) {
        return "";
    }
    XMLElement element("item", "http://jabber.org/protocol/pubsub#event");
    if (payload->getNode()) {
        element.setAttribute("node", *payload->getNode());
    }
    if (payload->getPublisher()) {
        element.setAttribute("publisher", *payload->getPublisher());
    }
    foreach(std::shared_ptr<Payload> item, payload->getData()) {
        element.addNode(std::make_shared<XMLRawTextNode>(serializers->getPayloadSerializer(item)->serialize(item)));
    }
    if (payload->getID()) {
        element.setAttribute("id", *payload->getID());
    }
    return element.serialize();
}


