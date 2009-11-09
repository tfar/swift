#include "Swiften/Client/Client.h"

#include <boost/bind.hpp>

#include "Swiften/Network/DomainNameResolver.h"
#include "Swiften/Network/MainBoostIOServiceThread.h"
#include "Swiften/Network/BoostIOServiceThread.h"
#include "Swiften/Client/ClientSession.h"
#include "Swiften/StreamStack/PlatformTLSLayerFactory.h"
#include "Swiften/Network/BoostConnectionFactory.h"
#include "Swiften/Network/DomainNameResolveException.h"
#include "Swiften/TLS/PKCS12Certificate.h"
#include "Swiften/Session/BasicSessionStream.h"

namespace Swift {

Client::Client(const JID& jid, const String& password) :
		IQRouter(this), jid_(jid), password_(password) {
	connectionFactory_ = new BoostConnectionFactory(&MainBoostIOServiceThread::getInstance().getIOService());
	tlsLayerFactory_ = new PlatformTLSLayerFactory();
}

Client::~Client() {
	delete tlsLayerFactory_;
	delete connectionFactory_;
}

bool Client::isAvailable() {
	return session_;
}

void Client::connect() {
	DomainNameResolver resolver;
	try {
		HostAddressPort remote = resolver.resolve(jid_.getDomain().getUTF8String());
		connection_ = connectionFactory_->createConnection();
		connection_->onConnectFinished.connect(boost::bind(&Client::handleConnectionConnectFinished, this, _1));
		connection_->connect(remote);
	}
	catch (const DomainNameResolveException& e) {
		onError(ClientError::DomainNameResolveError);
	}
}

void Client::handleConnectionConnectFinished(bool error) {
	if (error) {
		onError(ClientError::ConnectionError);
	}
	else {
		assert(!sessionStream_);
		sessionStream_ = boost::shared_ptr<BasicSessionStream>(new BasicSessionStream(connection_, &payloadParserFactories_, &payloadSerializers_, tlsLayerFactory_));
		sessionStream_->initialize();
		if (!certificate_.isEmpty()) {
			sessionStream_->setTLSCertificate(PKCS12Certificate(certificate_, password_));
		}
		//sessionStream_->onDataRead.connect(boost::bind(&Client::handleDataRead, this, _1));
		//sessionStream_->onDataWritten.connect(boost::bind(&Client::handleDataWritten, this, _1));

		session_ = boost::shared_ptr<ClientSession>(new ClientSession(jid_, sessionStream_));
		session_->onInitialized.connect(boost::bind(boost::ref(onConnected)));
		session_->onFinished.connect(boost::bind(&Client::handleSessionFinished, shared_from_this(), _1));
		session_->onNeedCredentials.connect(boost::bind(&Client::handleNeedCredentials, shared_from_this()));
		session_->onElementReceived.connect(boost::bind(&Client::handleElement, shared_from_this(), _1));
		session_->start();
	}
}

void Client::disconnect() {
	// TODO
	//if (session_) {
	//	session_->finishSession();
	//}
}

void Client::send(boost::shared_ptr<Stanza> stanza) {
	session_->sendElement(stanza);
}

void Client::sendIQ(boost::shared_ptr<IQ> iq) {
	send(iq);
}

void Client::sendMessage(boost::shared_ptr<Message> message) {
	send(message);
}

void Client::sendPresence(boost::shared_ptr<Presence> presence) {
	send(presence);
}

String Client::getNewIQID() {
	return idGenerator_.generateID();
}

void Client::handleElement(boost::shared_ptr<Element> element) {
	boost::shared_ptr<Message> message = boost::dynamic_pointer_cast<Message>(element);
	if (message) {
		onMessageReceived(message);
		return;
	}

	boost::shared_ptr<Presence> presence = boost::dynamic_pointer_cast<Presence>(element);
	if (presence) {
		onPresenceReceived(presence);
		return;
	}

	boost::shared_ptr<IQ> iq = boost::dynamic_pointer_cast<IQ>(element);
	if (iq) {
		onIQReceived(iq);
		return;
	}
}

void Client::setCertificate(const String& certificate) {
	certificate_ = certificate;
}

void Client::handleSessionFinished(boost::shared_ptr<Error> error) {
	if (error) {
		ClientError clientError;
		/*
		switch (*error) {
			case Session::ConnectionReadError:
				clientError = ClientError(ClientError::ConnectionReadError);
				break;
			case Session::ConnectionWriteError:
				clientError = ClientError(ClientError::ConnectionWriteError);
				break;
			case Session::XMLError:
				clientError = ClientError(ClientError::XMLError);
				break;
			case Session::AuthenticationFailedError:
				clientError = ClientError(ClientError::AuthenticationFailedError);
				break;
			case Session::NoSupportedAuthMechanismsError:
				clientError = ClientError(ClientError::NoSupportedAuthMechanismsError);
				break;
			case Session::UnexpectedElementError:
				clientError = ClientError(ClientError::UnexpectedElementError);
				break;
			case Session::ResourceBindError:
				clientError = ClientError(ClientError::ResourceBindError);
				break;
			case Session::SessionStartError:
				clientError = ClientError(ClientError::SessionStartError);
				break;
			case Session::TLSError:
				clientError = ClientError(ClientError::TLSError);
				break;
			case Session::ClientCertificateLoadError:
				clientError = ClientError(ClientError::ClientCertificateLoadError);
				break;
			case Session::ClientCertificateError:
				clientError = ClientError(ClientError::ClientCertificateError);
				break;
		}
		*/
		onError(clientError);
	}
}

void Client::handleNeedCredentials() {
	session_->sendCredentials(password_);
}

void Client::handleDataRead(const ByteArray& data) {
	onDataRead(String(data.getData(), data.getSize()));
}

void Client::handleDataWritten(const ByteArray& data) {
	onDataWritten(String(data.getData(), data.getSize()));
}

}
