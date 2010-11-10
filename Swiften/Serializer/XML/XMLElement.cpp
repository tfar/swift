/*
 * Copyright (c) 2010 Remko Tronçon
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include "Swiften/Serializer/XML/XMLElement.h"

#include "Swiften/Base/foreach.h"
#include "Swiften/Serializer/XML/XMLTextNode.h"

namespace Swift {

XMLElement::XMLElement(const String& tag, const String& xmlns, const String& text) : tag_(tag) {
	if (!xmlns.isEmpty()) {
		setAttribute("xmlns", xmlns);
	}
	if (!text.isEmpty()) {
		addNode(XMLTextNode::ref(new XMLTextNode(text)));
	}
}

String XMLElement::serialize() {
	String result;
	result += "<" + tag_;
	typedef std::pair<String,String> Pair;
	foreach(const Pair& p, attributes_) {
		result += " " + p.first + "=\"" + p.second + "\"";
	}

	if (childNodes_.size() > 0) {
		result += ">";
		foreach (boost::shared_ptr<XMLNode> node, childNodes_) {
			result += node->serialize();
		}
		result += "</" + tag_ + ">";
	}
	else {
		result += "/>";
	}
	return result;
}

void XMLElement::setAttribute(const String& attribute, const String& value) {
	String escapedValue(value);
	escapedValue.replaceAll('&', "&amp;");
	escapedValue.replaceAll('<', "&lt;");
	escapedValue.replaceAll('>', "&gt;");
	escapedValue.replaceAll('\'', "&apos;");
	escapedValue.replaceAll('"', "&quot;");
	attributes_[attribute] = escapedValue;
}

void XMLElement::addNode(boost::shared_ptr<XMLNode> node) {
	childNodes_.push_back(node);
}

}
