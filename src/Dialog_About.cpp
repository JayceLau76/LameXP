///////////////////////////////////////////////////////////////////////////////
// LameXP - Audio Encoder Front-End
// Copyright (C) 2004-2011 LoRd_MuldeR <MuldeR2@GMX.de>
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// http://www.gnu.org/licenses/gpl-2.0.txt
///////////////////////////////////////////////////////////////////////////////

#include "Dialog_About.h"

#include "Global.h"
#include "Resource.h"
#include "Model_Settings.h"

//Qt includes
#include <QDate>
#include <QApplication>
#include <QIcon>
#include <QPushButton>
#include <QDesktopServices>
#include <QUrl>
#include <QTimer>
#include <QFileInfo>
#include <QDir>

//Win32 includes
#include <Windows.h>

//Helper macros
#define LINK(URL) QString("<a href=\"%1\">%2</a>").arg(URL).arg(URL)
#define CONTRIBUTOR(LANG, CNTR, ICON) QString("<tr><td valign=\"middle\"><img src=\"%1\"></td><td>&nbsp;&nbsp;</td><td valign=\"middle\">%2</td><td>&nbsp;&nbsp;</td><td valign=\"middle\">%3</td></tr>").arg(ICON, LANG, CNTR);
#define VSTR(BASE,TOOL,FORMAT) BASE.arg(lamexp_version2string(FORMAT, lamexp_tool_version(TOOL)))

//Constants
const char *AboutDialog::neroAacUrl = "http://www.nero.com/eng/technologies-aac-codec.html";

////////////////////////////////////////////////////////////
// Constructor
////////////////////////////////////////////////////////////

AboutDialog::AboutDialog(SettingsModel *settings, QWidget *parent, bool firstStart)
:
	QMessageBox(parent),
	m_settings(settings)
{
	QString aboutText;

	aboutText += QString("<h2>%1</h2>").arg(tr("LameXP - Audio Encoder Front-end"));
	aboutText += QString("<b>Copyright (C) 2004-%1 LoRd_MuldeR &lt;MuldeR2@GMX.de&gt;. Some rights reserved.</b><br>").arg(max(lamexp_version_date().year(),QDate::currentDate().year()));
	aboutText += QString().sprintf("<b>Version %d.%02d %s, Build %d [%s]</b><br><br>", lamexp_version_major(), lamexp_version_minor(), lamexp_version_release(), lamexp_version_build(), lamexp_version_date().toString(Qt::ISODate).toLatin1().constData());
	aboutText += QString("<nobr>%1</nobr><br>").arg(tr("Please visit %1 for news and updates!").arg(LINK("http://forum.doom9.org/showthread.php?t=157726")));
	aboutText += "<hr><br>";
	aboutText += "<nobr><tt>This program is free software; you can redistribute it and/or<br>";
	aboutText += "modify it under the terms of the GNU General Public License<br>";
	aboutText += "as published by the Free Software Foundation; either version 2<br>";
	aboutText += "of the License, or (at your option) any later version.<br><br>";
	aboutText += "This program is distributed in the hope that it will be useful,<br>";
	aboutText += "but WITHOUT ANY WARRANTY; without even the implied warranty of<br>";
	aboutText += "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the<br>";
	aboutText += "GNU General Public License for more details.<br><br>";
	aboutText += "You should have received a copy of the GNU General Public License<br>";
	aboutText += "along with this program; if not, write to the Free Software<br>";
	aboutText += "Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.</tt></nobr><br>";
	aboutText += "<hr><table><tr>";
	aboutText += "<td valign=\"middle\"><img src=\":/icons/error_big.png\"</td><td>&nbsp;</td>";
	aboutText += QString("<td><font color=\"darkred\">%1</font></td>").arg(tr("Note: LameXP is free software. Do <b>not</b> pay money to obtain or use LameXP! If some third-party website tries to make you pay for downloading LameXP, you should <b>not</b> respond to the offer !!!"));
	aboutText += "</tr></table><hr><br>";
	aboutText += QString("%1<br>").arg(tr("Special thanks go out to \"John33\" from %1 for his continuous support.").arg(LINK("http://www.rarewares.org/")));
	
	setText(aboutText);
	setIconPixmap(dynamic_cast<QApplication*>(QApplication::instance())->windowIcon().pixmap(QSize(64,64)));
	setWindowTitle(tr("About LameXP"));
	
	if(firstStart)
	{
		QPushButton *firstButton = addButton(tr("Show License Text"), QMessageBox::AcceptRole);
		firstButton->setIcon(QIcon(":/icons/script.png"));
		firstButton->setMinimumWidth(135);
		firstButton->disconnect();
		connect(firstButton, SIGNAL(clicked()), this, SLOT(openLicenseText()));

		QPushButton *secondButton = addButton(tr("Accept License"), QMessageBox::AcceptRole);
		secondButton->setIcon(QIcon(":/icons/accept.png"));
		secondButton->setMinimumWidth(120);

		QPushButton *thirdButton = addButton(tr("Decline License"), QMessageBox::AcceptRole);
		thirdButton->setIcon(QIcon(":/icons/delete.png"));
		thirdButton->setMinimumWidth(120);
		thirdButton->setEnabled(false);
	}
	else
	{
		QPushButton *firstButton = addButton(tr("3rd Party S/W"), QMessageBox::AcceptRole);
		firstButton->setIcon(QIcon(":/icons/page_white_cplusplus.png"));
		firstButton->setMinimumWidth(120);
		firstButton->disconnect();
		connect(firstButton, SIGNAL(clicked()), this, SLOT(showMoreAbout()));

		QPushButton *secondButton = addButton(tr("Contributors"), QMessageBox::AcceptRole);
		secondButton->setIcon(QIcon(":icons/user_suit.png"));
		secondButton->setMinimumWidth(120);
		secondButton->disconnect();
		connect(secondButton, SIGNAL(clicked()), this, SLOT(showAboutContributors()));

		QPushButton *thirdButton = addButton(tr("About Qt4"), QMessageBox::AcceptRole);
		thirdButton->setIcon(QIcon(":/images/Qt.svg"));
		thirdButton->setMinimumWidth(120);
		thirdButton->disconnect();
		connect(thirdButton, SIGNAL(clicked()), this, SLOT(showAboutQt()));

		QPushButton *fourthButton = addButton(tr("Discard"), QMessageBox::AcceptRole);
		fourthButton->setIcon(QIcon(":/icons/cross.png"));
		fourthButton->setMinimumWidth(90);
	}

	m_firstShow = firstStart;
}

AboutDialog::~AboutDialog(void)
{
}

////////////////////////////////////////////////////////////
// Public Functions
////////////////////////////////////////////////////////////

int AboutDialog::exec()
{
	if(m_settings->soundsEnabled())
	{
		if(!m_firstShow || !playResoureSound("imageres.dll", 5080, true))
		{
			PlaySound(MAKEINTRESOURCE(IDR_WAVE_ABOUT), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
		}
	}
	
	switch(QMessageBox::exec())
	{
	case 1:
		return 1;
		break;
	case 2:
		return -1;
		break;
	default:
		return 0;
		break;
	}
}

////////////////////////////////////////////////////////////
// Slots
////////////////////////////////////////////////////////////

void AboutDialog::enableButtons(void)
{
	const QList<QAbstractButton*> buttonList = buttons();
	
	for(int i = 0; i < buttonList.count(); i++)
	{
		buttonList.at(i)->setEnabled(true);
	}

	setCursor(QCursor(Qt::ArrowCursor));
}

void AboutDialog::openLicenseText(void)
{
	QDesktopServices::openUrl(QUrl("http://www.gnu.org/licenses/gpl-2.0.txt"));
}

void AboutDialog::showAboutQt(void)
{
	QMessageBox::aboutQt(this);
}

void AboutDialog::showAboutContributors(void)
{
	QString contributorsAboutText;
	contributorsAboutText += QString("<h3><nobr>%1</nobr></h3>").arg(tr("The following people have contributed to LameXP:"));
	contributorsAboutText += QString("<b>%1</b>").arg(tr("Translators:"));
	contributorsAboutText += "<table style=\"margin-top:5px\">";
	contributorsAboutText += CONTRIBUTOR("Englisch", "LoRd_MuldeR &lt;MuldeR2@GMX.de&gt;", ":/flags/en.png");
	contributorsAboutText += CONTRIBUTOR("Deutsch", "LoRd_MuldeR &lt;MuldeR2@GMX.de&gt;", ":/flags/de.png");
	contributorsAboutText += "</table>";
	contributorsAboutText += "<br><br>";
	contributorsAboutText += QString("<nobr><i>%1</i></nobr><br>").arg(tr("If you are willing to contribute a LameXP translation, feel free to contact us!"));

	QMessageBox *contributorsAboutBox = new QMessageBox(this);
	contributorsAboutBox->setText(contributorsAboutText);
	contributorsAboutBox->setIconPixmap(dynamic_cast<QApplication*>(QApplication::instance())->windowIcon().pixmap(QSize(64,64)));

	QPushButton *closeButton = contributorsAboutBox->addButton(tr("Discard"), QMessageBox::AcceptRole);
	closeButton->setIcon(QIcon(":/icons/cross.png"));
	closeButton->setMinimumWidth(90);

	contributorsAboutBox->setWindowTitle(tr("About Contributors"));
	contributorsAboutBox->setIconPixmap(QIcon(":/images/Logo_Contributors.png").pixmap(QSize(64,74)));
	contributorsAboutBox->setWindowIcon(QIcon(":/icons/user_suit.png"));
	contributorsAboutBox->exec();
				
	LAMEXP_DELETE(contributorsAboutBox);
}

void AboutDialog::showMoreAbout(void)
{
	QString moreAboutText;
	moreAboutText += QString("<h3>%1</h3>").arg(tr("The following third-party software is used in LameXP:"));
	moreAboutText += "<div style=\"margin-left:-25px;font-size:8pt\"><ul>";
	
	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("LAME - OpenSource mp3 Encoder")), "lame.exe", "v?.?? a??");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Released under the terms of the GNU Leser General Public License."));
	moreAboutText += LINK("http://lame.sourceforge.net/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";
	
	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("OggEnc - Ogg Vorbis Encoder")), "oggenc2_i386.exe", "v?.??");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Completely open and patent-free audio encoding technology."));
	moreAboutText += LINK("http://www.vorbis.com/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";
	
	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("Nero AAC reference MPEG-4 Encoder")), "neroAacEnc.exe", "v?.?.?.?");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Freeware state-of-the-art HE-AAC encoder with 2-Pass support."));
	moreAboutText += QString("<nobr><i>%1</i></nobr><br>").arg(tr("Available from vendor web-site as free download:"));
	moreAboutText += LINK(neroAacUrl);
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";
	
	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("FLAC - Free Lossless Audio Codec")), "flac.exe", "v?.?.?");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Open and patent-free lossless audio compression technology."));
	moreAboutText += LINK("http://flac.sourceforge.net/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";

	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("AC3Filter Tools - AC3/DTS Decoder")), "valdec.exe", "v?.??");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Released under the terms of the GNU Leser General Public License."));
	moreAboutText += LINK("http://www.ac3filter.net/projects/tools");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";

	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("MediaInfo - Media File Analysis Tool")), "mediainfo_i386.exe", "v?.?.??");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Released under the terms of the GNU Leser General Public License."));
	moreAboutText += LINK("http://mediainfo.sourceforge.net/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";

	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("SoX - Sound eXchange")), "sox.exe", "v??.?.?");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Released under the terms of the GNU Leser General Public License."));
	moreAboutText += LINK("http://sox.sourceforge.net/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";
	
	moreAboutText += VSTR(QString("<li><b>%1 (%2)</b><br>").arg(tr("GnuPG - The GNU Privacy Guard")), "gpgv.exe", "v?.?.??");
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("Released under the terms of the GNU Leser General Public License."));
	moreAboutText += LINK("http://www.gnupg.org/");
	moreAboutText += "<div style=\"font-size:1pt\"><br></div>";

	moreAboutText += QString("<li><b>%1 (v1.3)</b><br>").arg(tr("Silk Icons - Over 700  icons in PNG format"));
	moreAboutText += QString("<nobr>%1</nobr><br>").arg(tr("By Mark James, released under the Creative Commons 'by' License."));
	moreAboutText += LINK("http://www.famfamfam.com/lab/icons/silk/");
	moreAboutText += "<br></ul></div>";

	QMessageBox *moreAboutBox = new QMessageBox(this);
	moreAboutBox->setText(moreAboutText);
	moreAboutBox->setIconPixmap(dynamic_cast<QApplication*>(QApplication::instance())->windowIcon().pixmap(QSize(64,64)));

	QPushButton *closeButton = moreAboutBox->addButton(tr("Discard"), QMessageBox::AcceptRole);
	closeButton->setIcon(QIcon(":/icons/cross.png"));
	closeButton->setMinimumWidth(90);

	moreAboutBox->setWindowTitle(tr("About Third-party Software"));
	moreAboutBox->setIconPixmap(QIcon(":/images/Logo_Software.png").pixmap(QSize(64,71)));
	moreAboutBox->setWindowIcon(QIcon(":/icons/page_white_cplusplus.png"));
	moreAboutBox->exec();
				
	LAMEXP_DELETE(moreAboutBox);
}

////////////////////////////////////////////////////////////
// Protected Functions
////////////////////////////////////////////////////////////

void AboutDialog::showEvent(QShowEvent *e)
{
	QDialog::showEvent(e);
	if(m_firstShow)
	{
		const QList<QAbstractButton*> buttonList = buttons();

		for(int i = 1; i < buttonList.count(); i++)
		{
			buttonList.at(i)->setEnabled(false);
		}

		QTimer::singleShot(5000, this, SLOT(enableButtons()));
		setCursor(QCursor(Qt::WaitCursor));
	}
}

////////////////////////////////////////////////////////////
// Private Functions
////////////////////////////////////////////////////////////

bool AboutDialog::playResoureSound(const QString &library, const unsigned long soundId, const bool async)
{
	HMODULE module = 0;
	DWORD flags = SND_RESOURCE;
	bool result = false;

	QFileInfo libraryFile(library);
	if(!libraryFile.isAbsolute())
	{
		unsigned int buffSize = GetSystemDirectoryW(NULL, NULL) + 1;
		wchar_t *buffer = (wchar_t*) _malloca(buffSize * sizeof(wchar_t));
		unsigned int result = GetSystemDirectory(buffer, buffSize);
		if(result > 0 && result < buffSize)
		{
			libraryFile.setFile(QString("%1/%2").arg(QDir::fromNativeSeparators(QString::fromUtf16(reinterpret_cast<const unsigned short*>(buffer))), library));
		}
		_freea(buffer);
	}

	if(async)
	{
		flags |= SND_ASYNC;
	}
	else
	{
		flags |= SND_SYNC;
	}

	module = LoadLibraryW(reinterpret_cast<const wchar_t*>(QDir::toNativeSeparators(libraryFile.absoluteFilePath()).utf16()));

	if(module)
	{
		result = PlaySound((LPCTSTR) soundId, module, flags);
		FreeLibrary(module);
	}

	return result;
}