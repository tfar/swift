/*
 * Copyright (c) 2012 Remko Tronçon
 * Licensed under the GNU General Public License v3.
 * See Documentation/Licenses/GPLv3.txt for more information.
 */

#include "Swift/QtUI/QtConnectionSettingsWindow.h"

#include <boost/lexical_cast.hpp>

#include <QCoreApplication>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>
#include <QtGlobal>
#include <QPushButton>
#include <QTextEdit>
#include <QFile>
#include <QTextStream>
#include <Swift/QtUI/QtSwiftUtil.h>

namespace Swift {

QtConnectionSettingsWindow::QtConnectionSettingsWindow(const ClientOptions& options) : QDialog() {
	ui.setupUi(this);

	connect(ui.connectionMethod, SIGNAL(currentIndexChanged(int)), ui.stackedWidget, SLOT(setCurrentIndex(int)));

	connect(ui.manual_manualHost, SIGNAL(toggled(bool)), ui.manual_manualHostNameLabel, SLOT(setEnabled(bool)));
	connect(ui.manual_manualHost, SIGNAL(toggled(bool)), ui.manual_manualHostName, SLOT(setEnabled(bool)));
	connect(ui.manual_manualHost, SIGNAL(toggled(bool)), ui.manual_manualHostPortLabel, SLOT(setEnabled(bool)));
	connect(ui.manual_manualHost, SIGNAL(toggled(bool)), ui.manual_manualHostPort, SLOT(setEnabled(bool)));

	connect(ui.manual_manualProxy, SIGNAL(toggled(bool)), ui.manual_manualProxyHostLabel, SLOT(setEnabled(bool)));
	connect(ui.manual_manualProxy, SIGNAL(toggled(bool)), ui.manual_manualProxyHost, SLOT(setEnabled(bool)));
	connect(ui.manual_manualProxy, SIGNAL(toggled(bool)), ui.manual_manualProxyPortLabel, SLOT(setEnabled(bool)));
	connect(ui.manual_manualProxy, SIGNAL(toggled(bool)), ui.manual_manualProxyPort, SLOT(setEnabled(bool)));

	connect(ui.bosh_manualProxy, SIGNAL(toggled(bool)), ui.bosh_manualProxyHostLabel, SLOT(setEnabled(bool)));
	connect(ui.bosh_manualProxy, SIGNAL(toggled(bool)), ui.bosh_manualProxyHost, SLOT(setEnabled(bool)));
	connect(ui.bosh_manualProxy, SIGNAL(toggled(bool)), ui.bosh_manualProxyPortLabel, SLOT(setEnabled(bool)));
	connect(ui.bosh_manualProxy, SIGNAL(toggled(bool)), ui.bosh_manualProxyPort, SLOT(setEnabled(bool)));

	connect(ui.manual_proxyType, SIGNAL(currentIndexChanged(int)), SLOT(handleProxyTypeChanged(int)));

	ui.manual_useTLS->setCurrentIndex(2);

	ui.manual_proxyType->setCurrentIndex(0);

	ClientOptions defaults;
	if (options.boshURL.empty()) {
		int i = 0;
		bool isDefault = options.useStreamCompression == defaults.useStreamCompression;
		isDefault &= options.useTLS == defaults.useTLS;
		isDefault &= options.allowPLAINWithoutTLS == defaults.allowPLAINWithoutTLS;
		isDefault &= options.useStreamCompression == defaults.useStreamCompression;
		isDefault &= options.useAcks == defaults.useAcks;
		isDefault &= options.manualHostname == defaults.manualHostname;
		isDefault &= options.manualPort == defaults.manualPort;
		isDefault &= options.proxyType == defaults.proxyType;
		isDefault &= options.manualProxyHostname == defaults.manualProxyHostname;
		isDefault &= options.manualProxyPort == defaults.manualProxyPort;
		if (isDefault) {
		    ui.connectionMethod->setCurrentIndex(0);
		}
		else {
			ui.connectionMethod->setCurrentIndex(1);
			ui.manual_useTLS->setCurrentIndex(options.useTLS);
			ui.manual_allowPLAINWithoutTLS->setChecked(options.allowPLAINWithoutTLS);
			ui.manual_allowCompression->setChecked(options.useStreamCompression);
			if (!options.manualHostname.empty()) {
				ui.manual_manualHost->setChecked(true);
				ui.manual_manualHostName->setText(P2QSTRING(options.manualHostname));
				ui.manual_manualHostPort->setText(P2QSTRING(boost::lexical_cast<std::string>(options.manualPort)));
			}
			ui.manual_proxyType->setCurrentIndex(options.proxyType);
			if (!options.manualProxyHostname.empty()) {
				ui.manual_manualProxy->setChecked(true);
				ui.manual_manualProxyHost->setText(P2QSTRING(options.manualProxyHostname));
				ui.manual_manualHostPort->setText(P2QSTRING(boost::lexical_cast<std::string>(options.manualProxyPort)));
			}
		}
	} else {
		ui.connectionMethod->setCurrentIndex(2);
		ui.bosh_uri->setText(P2QSTRING(options.boshURL.toString()));
		if (!options.boshHTTPConnectProxyURL.empty()) {
			ui.bosh_manualProxy->setChecked(true);
			ui.bosh_manualProxyHost->setText(P2QSTRING(options.boshHTTPConnectProxyURL.getHost()));
			ui.bosh_manualProxyPort->setText(P2QSTRING(boost::lexical_cast<std::string>(options.boshHTTPConnectProxyURL.getPort())));
		}
	}
}

void QtConnectionSettingsWindow::handleProxyTypeChanged(int index) {
	bool proxySettingsVisible = index != NoProxy && index != SystemProxy;
	ui.manual_manualProxy->setVisible(proxySettingsVisible);
	ui.manual_manualProxyHostLabel->setVisible(proxySettingsVisible);
	ui.manual_manualProxyHost->setVisible(proxySettingsVisible);
	ui.manual_manualProxyPortLabel->setVisible(proxySettingsVisible);
	ui.manual_manualProxyPort->setVisible(proxySettingsVisible);
}

ClientOptions QtConnectionSettingsWindow::getOptions() {
	ClientOptions options;
	if (ui.connectionMethod->currentIndex() > 0) {
		/* Not automatic */
		if (ui.connectionMethod->currentIndex() == 1) {
			/* Manual */
			options.useTLS = static_cast<ClientOptions::UseTLS>(ui.manual_useTLS->currentIndex());
			options.useStreamCompression = ui.manual_allowCompression->isChecked();
			options.allowPLAINWithoutTLS = ui.manual_allowPLAINWithoutTLS->isChecked();
			if (ui.manual_manualHost->isChecked()) {
				options.manualHostname = Q2PSTRING(ui.manual_manualHostName->text());
				try {
					options.manualPort = boost::lexical_cast<int>(Q2PSTRING(ui.manual_manualHostPort->text()));
				} catch (const boost::bad_lexical_cast&) {}
			}
			options.proxyType = static_cast<ClientOptions::ProxyType>(ui.manual_proxyType->currentIndex());
			if (ui.manual_manualProxy->isChecked()) {
				options.manualProxyHostname = Q2PSTRING(ui.manual_manualProxyHost->text());
				try {
					options.manualProxyPort = boost::lexical_cast<int>(Q2PSTRING(ui.manual_manualProxyPort->text()));
				} catch (const boost::bad_lexical_cast&) {}
			}
		}
		else {
			/* BOSH */
			options.boshURL = URL(Q2PSTRING(ui.bosh_uri->text()));
			if (ui.bosh_manualProxy->isChecked()) {
				std::string host = Q2PSTRING(ui.bosh_manualProxyHost->text());
				int port = 80;
				try {
					port = boost::lexical_cast<int>(Q2PSTRING(ui.bosh_manualProxyPort->text()));
				} catch (const boost::bad_lexical_cast&) {}
				options.boshHTTPConnectProxyURL = URL("http", host, port, "");
			}
		}
	}
	return options;
}

}