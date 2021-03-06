/*
 * Copyright (c) 2012 Maciej Niedzielski
 * Licensed under the simplified BSD license.
 * See Documentation/Licenses/BSD-simplified.txt for more information.
 */

/*
 * Copyright (c) 2014-2015 Isode Limited.
 * All rights reserved.
 * See the COPYING file for more information.
 */

#include <Swift/Controllers/HighlightRule.h>

#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/lambda/lambda.hpp>

#include <Swiften/Base/Regex.h>
#include <Swiften/Base/foreach.h>

namespace Swift {

HighlightRule::HighlightRule()
    : nickIsKeyword_(false)
    , matchCase_(false)
    , matchWholeWords_(false)
    , matchChat_(false)
    , matchMUC_(false)
{
}

boost::regex HighlightRule::regexFromString(const std::string & s) const
{
    std::string escaped = Regex::escape(s);
    std::string word = matchWholeWords_ ? "\\b" : "";
    boost::regex::flag_type flags = boost::regex::normal;
    if (!matchCase_) {
        flags |= boost::regex::icase;
    }
    return boost::regex(word + escaped + word, flags);
}

void HighlightRule::updateRegex() const
{
    keywordRegex_.clear();
    foreach (const std::string & k, keywords_) {
        keywordRegex_.push_back(regexFromString(k));
    }
    senderRegex_.clear();
    foreach (const std::string & s, senders_) {
        senderRegex_.push_back(regexFromString(s));
    }
}

std::string HighlightRule::boolToString(bool b)
{
    return b ? "1" : "0";
}

bool HighlightRule::boolFromString(const std::string& s)
{
    return s == "1";
}

bool HighlightRule::isMatch(const std::string& body, const std::string& sender, const std::string& nick, MessageType messageType) const
{
    if ((messageType == HighlightRule::ChatMessage && matchChat_) || (messageType == HighlightRule::MUCMessage && matchMUC_)) {

        bool matchesKeyword = keywords_.empty() && (nick.empty() || !nickIsKeyword_);
        bool matchesSender = senders_.empty();

        if (!matchesKeyword) {
            // check if the nickname matches
            if (nickIsKeyword_ && !nick.empty() && boost::regex_search(body, regexFromString(nick))) {
                matchesKeyword = true;
            }

            // check if a keyword matches
            if (!matchesKeyword && !keywords_.empty()) {
                foreach (const boost::regex &keyword, keywordRegex_) {
                    if (boost::regex_search(body, keyword)) {
                        matchesKeyword = true;
                        break;
                    }
                }
            }
        }

        foreach (const boost::regex & rx, senderRegex_) {
            if (boost::regex_search(sender, rx)) {
                matchesSender = true;
                break;
            }
        }

        if (matchesKeyword && matchesSender) {
            return true;
        }
    }

    return false;
}

void HighlightRule::setSenders(const std::vector<std::string>& senders)
{
    senders_ = senders;
    updateRegex();
}

void HighlightRule::setKeywords(const std::vector<std::string>& keywords)
{
    keywords_ = keywords;
    updateRegex();
}

std::vector<boost::regex> HighlightRule::getKeywordRegex(const std::string& nick) const {
    if (nickIsKeyword_) {
        std::vector<boost::regex> regex;
        if (!nick.empty()) {
            regex.push_back(regexFromString(nick));
        }
        return regex;
    } else {
        return keywordRegex_;
    }
}

void HighlightRule::setNickIsKeyword(bool nickIsKeyword)
{
    nickIsKeyword_ = nickIsKeyword;
    updateRegex();
}

void HighlightRule::setMatchCase(bool matchCase)
{
    matchCase_ = matchCase;
    updateRegex();
}

void HighlightRule::setMatchWholeWords(bool matchWholeWords)
{
    matchWholeWords_ = matchWholeWords;
    updateRegex();
}

void HighlightRule::setMatchChat(bool matchChat)
{
    matchChat_ = matchChat;
    updateRegex();
}

void HighlightRule::setMatchMUC(bool matchMUC)
{
    matchMUC_ = matchMUC;
    updateRegex();
}

bool HighlightRule::isEmpty() const
{
    return senders_.empty() && keywords_.empty() && !nickIsKeyword_ && !matchChat_ && !matchMUC_ && action_.isEmpty();
}

bool operator ==(HighlightRule const& a, HighlightRule const& b) {
    if (a.getSenders() != b.getSenders()) {
        return false;
    }

    if (a.getKeywords() != b.getKeywords()) {
        return false;
    }

    if (a.getNickIsKeyword() != b.getNickIsKeyword()) {
        return false;
    }

    if (a.getMatchChat() != b.getMatchChat()) {
        return false;
    }

    if (a.getMatchMUC() != b.getMatchMUC()) {
        return false;
    }

    if (a.getMatchCase() != b.getMatchCase()) {
        return false;
    }

    if (a.getMatchWholeWords() != b.getMatchWholeWords()) {
        return false;
    }

    if (a.getAction() != b.getAction()) {
        return false;
    }

    return true;
}

}
