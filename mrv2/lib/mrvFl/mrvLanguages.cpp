// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#else
#  include <unistd.h>
#endif


#include <FL/fl_ask.H>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvFl/mrvLanguages.h"
#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"

#ifdef _WIN32
#  define execv _execv
#endif

namespace {
    const char* kModule = "lang";
}

LanguageTable kLanguages[18] = {
    { 16, "ar.UTF-8" },
    {  0, "cs.UTF-8" },
    {  1, "de.UTF-8" },
    {  2, "en.UTF-8" },
    {  3, "es.UTF-8" },
    {  4, "fr.UTF-8" },
    { 15, "gr.UTF-8" },
    {  5, "it.UTF-8" },
    {  6, "ja.UTF-8" },
    {  7, "ko.UTF-8" },
    { 17, "nl.UTF-8" },
    {  8, "pl.UTF-8" },
    {  9, "pt.UTF-8" },
    { 10, "ro.UTF-8" },
    { 11, "ru.UTF-8" },
    { 14, "sv.UTF-8" },
    { 12, "tr.UTF-8" },
    { 13, "zh.UTF-8" },
};



void check_language( PreferencesUI* uiPrefs, int& language_index )
{
    int uiIndex = uiPrefs->uiLanguage->value();
    int index = kLanguages[uiIndex].index;
    if ( index != language_index )
    {
        int ok = fl_choice( _("Need to reboot mrv2 to change language.  "
                              "Are you sure you want to continue?" ),
                            _("No"),  _("Yes"), NULL, NULL );
        if ( ok )
        {
            language_index = index;
            const char* language = kLanguages[uiIndex].code;

            setenv( "LC_CTYPE", "UTF-8", 1 );
            setenv( "LANGUAGE", language, 1 );

            //
            // Windows blows, it does not pay attention to the LC_MESSAGES
            // It only reads LANGUAGE
            //
#if 0
            char buf[128];
            // We change the system language environment variable so that
            // the next time we start we start with the same language.
            // Saving the setting in the preferences is not enough on Windows.
            snprintf( buf, 128, "setx LANGUAGE %s", language );

            STARTUPINFO si;
            PROCESS_INFORMATION pi;

            ZeroMemory( &si, sizeof(si) );
            si.cb = sizeof(si);
            ZeroMemory( &pi, sizeof(pi) );

            // Start the child process.
            if( !CreateProcess( NULL,   // No module name (use command line)
								buf,    // Command line
                                NULL,   // Process handle not inheritable
                                NULL,   // Thread handle not inheritable
                                FALSE,  // Set handle inheritance to FALSE
                                CREATE_NO_WINDOW,  // No console window
                                NULL,   // Use parent's environment block
                                NULL,   // Use parent's starting directory
                                &si,    // Pointer to STARTUPINFO structure
                                &pi )   // Pointer to PROCESS_INFORMATION struct
                )
            {
                LOG_ERROR( "CreateProcess failed" );
                return;
            }

            // Wait until child process exits.
            WaitForSingleObject( pi.hProcess, INFINITE );

            // Close process and thread handles.
            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );
#endif

            Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                 "mrv2" );

            // Save ui preferences
            Fl_Preferences ui( base, "ui" );
            ui.set( "language", language_index );

            base.flush();

            // deleete ViewerUI
            delete mrv::Preferences::ui;

            std::string root = getenv( "MRV_ROOT" );
            root += "/bin/mrv2";

            const char *const parmList[] = {root.c_str(), NULL};
            execv( root.c_str(), (char* const*) parmList );
        }
        else
        {
            uiPrefs->uiLanguage->value( language_index );
        }
    }

}


char* select_character( const char* p, bool colon )
{
    int size = fl_utf8len1(p[0]);
    const char* end = p + size;

    if ( mrv::Preferences::language_index == 16 && size > 1 )
    {
        p += size - 2; // Arabic, right letter
    }

    int len;
    unsigned code;
    if (*p & 0x80) {              // what should be a multibyte encoding
        code = fl_utf8decode(p,end,&len);
        if (len<2) code = 0xFFFD; // Turn errors into REPLACEMENT CHARACTER
    } else {                      // handle the 1-byte UTF-8 encoding:
        code = *p;
        len = 1;
    }


    static char buf[8];
    memset( buf, 0, 7 );
    if ( len > 1 )
    {
        len = fl_utf8encode( code, buf );
    }
    else
    {
        buf[0] = *p;
    }
    if ( colon )
    {
        buf[len] = ':';
        ++len;
    }
    buf[len] = 0;

    return buf;
}


void select_character( mrv::PopupMenu* o, bool colon )
{
    int i = o->value();
    if ( i < 0 ) return;
    const char* p = o->text(i);
    o->copy_label( select_character( p, colon ) );
}

namespace mrv
{

    void initLocale(const char* code)
    {
		setlocale(LC_ALL, code);

		const char* defaultLanguage = setlocale(LC_ALL, NULL);

		const char* language = getenv( "LANGUAGE" );
		
		// Needed for Linux
		setenv( "LANGUAGE", code, 1 );

			
#ifdef _WIN32
		//
		// On Windows, the environment variable (LANGUAGE in our case), does
		// not get propagated to libint.dll.  Therefore, we restart mrv2
		// again after we set the LANGUAGE var and libintl will *then*
		// pick up the variable.
		//
		if ( ! language || strcmp(language, code) != 0 )
		{
            // deleete ViewerUI
            delete mrv::Preferences::ui;			
			
			int argc = 0;
			LPWSTR cmdLine = GetCommandLineW();
			LPWSTR* argv = CommandLineToArgvW(cmdLine, &argc);

			
            intptr_t ret = _wexecv( argv[0], argv );
			
			LocalFree(argv);
			if ( ret == -1 )
			{
				std::cerr << "_wexec failed with " << errno << std::endl;
				std::cerr << strerror(errno) << std::endl;
				exit(1);
			}
			exit(0);
		}
#endif
		
#ifdef __APPLE__
		setenv( "LC_NUMERIC", code, 1 );
		setenv( "LC_MESSAGES", code, 1 );
#endif
    }


    void setLanguageLocale()
    {
#if defined __APPLE__
        setenv( "LC_CTYPE",  "UTF-8", 1 );
#endif

        int lang = -1;
        const char* code = "C";
        const char* t;
        {
            Fl_Preferences base( mrv::prefspath().c_str(), "filmaura",
                                 "mrv2" );

            // Load ui language preferences
            Fl_Preferences ui( base, "ui" );

            ui.get( "language", lang, -1 );
            if ( lang >= 0 )
            {
                for ( unsigned i = 0;
                      i < sizeof(kLanguages) / sizeof(LanguageTable); ++i)
                {
                    if ( kLanguages[i].index == lang )
                    {
                        code = kLanguages[i].code;
                        break;
                    }
                }

                initLocale(code);
            }
        }

        const char* numericLocale;
        if ( lang < 0 )
            numericLocale = setlocale(LC_ALL, "");
        else
        {
            numericLocale = setlocale(LC_ALL, NULL);
        }

#if defined __APPLE__ && defined __MACH__
        numericLocale = setlocale( LC_MESSAGES, NULL );
#endif

        const char* language = getenv( "LANGUAGE" );
        if ( !language || language[0] == '\0' ) language = getenv( "LC_ALL" );
        if ( !language || language[0] == '\0' ) language = getenv( "LC_NUMERIC" );
        if ( !language || language[0] == '\0' ) language = getenv( "LANG" );
        if ( language )
        {
            // THis is for Apple mainly, as it we just set LC_MESSAGES only
            // and not the numeric locale, which we must set separately for
            // those locales that use periods in their floating point.
            if ( strcmp( language, "C" ) == 0 ||
                 strncmp( language, "ar", 2 ) == 0 ||
                 strncmp( language, "en", 2 ) == 0 ||
                 strncmp( language, "ja", 2 ) == 0 ||
                 strncmp( language, "ko", 2 ) == 0 ||
                 strncmp( language, "zh", 2 ) == 0 )
                numericLocale = "C";
        }

        setlocale( LC_NUMERIC, numericLocale );
		
        // Create and install global locale
        fs::path::imbue(std::locale());

        std::string path = fl_getenv("MRV_ROOT");
        path += "/share/locale/";

        char buf[256];
        sprintf( buf, "mrv2-v%s", mrv::version() );
        bindtextdomain(buf, path.c_str() );
        bind_textdomain_codeset(buf, "UTF-8" );
        textdomain(buf);

        LOG_INFO( _("Translations: ") << path );
    }

}
