#!/usr/local/bin/perl5

push(@INC, "/home/xml4c/bin", "/home/xml4c/bin/perl/perl-RUN/opt/perl5/lib", "/Development2/cupert/bin/perl/perl-RUN/opt/perl5/lib", "/Development/cupert/usr/local/perl/perl-RUN/opt/perl5/lib");

require "getopt.pl";
require 5.0;

$|=1;   # Force a flush after every print


# Extract the source and target directories
&Getopt('sopcxmntrj');
$XERCESCROOT = $opt_s;
$targetdir = $opt_o;

# Check for the environment variables and exit if error
if (!length($XERCESCROOT) || !length($targetdir) || (length($opt_h) > 0) ) {
    print ("Usage is: packageBinaries <options>\n");
    print ("  where options are:\n");
    print ("    -s <source_directory>\n");
    print ("    -o <target_directory>\n");
    print ("    -c <C compiler name> (e.g. gcc, cc or xlc_r)\n");
    print ("    -x <C++ compiler name> (e.g. g++, CC, aCC, c++ or xlC_r)\n");
    print ("    -m <message loader> can be 'inmem' \(default\), 'icu' or 'iconv'\n");
    print ("    -n <net accessor> can be 'fileonly' or 'socket' \(default\)\n");
    print ("    -t <transcoder> can be 'icu' or 'native' \(default\)\n");
    print ("    -r <thread option> can be 'pthread' \(default\)or 'dce' (only used on HP-11)\n");
    print ("    -j suppress building of ICU (speeds up builds when debugging)\n");
    print ("    -h to get help on these commands\n\n");
    print ("Example: Under unix's\n");
    print ("    perl packageBinaries.pl -s \$HOME/xerces-c-src_1_3_0");
    print (" -o \$HOME/xerces-c_1_3_0-linux -c gcc -x g++ -m inmem -n fileonly -t native\n\n");
    print ("Example: Under Windows\n");
    print ("    perl packageBinaries.pl -s \\xerces-c-src_1_3_0");
    print (" -o\\xerces-c_1_3_0-win32 [-n fileonly] [-t icu]\n\n");
    print ("Note:\n");
    print ("    Under Windows, by default the XercesLib project files is\n");
    print ("    configured to use Win32 resource file based message loader,\n");
    print ("    WinSocket based net accessor and native Win32 based transcoder.\n");
    print ("    The two options mentioned in the example above are the only\n");
    print ("    options relevant under Windows on the command line for this script.\n");
    exit(-1);
}

# Set up the environment variables for ICU
# As of Version 3, ICU is not a required component of XERCES-C
$ICUROOT = $ENV{'ICUROOT'};
if (!length($ICUROOT)) {
    print "You have not defined your ICU install directory.\n";
    print "To build with ICU, you must set an environment variable called ICUROOT\n";
    print "Proceeding to build XERCES-C without ICU...\n";
}
  
# Check if the source directory exists or not
if (!(-e $XERCESCROOT)) {
    print ("The directory $XERCESCROOT does not exist. Cannot proceed any further.\n");
    exit(-1);
}

# Check if the target directory exists, exit if it does
if (-e $targetdir) {
    print ("Error: The target directory \'$targetdir\' already exists.\n");
    print ("       You must start with a clean directory to package your product.\n");
    exit(1);
}

#Fix the backslashes on the Windows platform
$XERCESCROOT =~ s/\\/\//g;
$ICUROOT =~ s/\\/\//g;
$targetdir =~ s/\\/\//g;

# Find out the platform from 'uname -a'
open(PLATFORM, "uname -s|");
$platform = <PLATFORM>;
chomp($platform);
close (PLATFORM);

      
print "Packaging binaries for \`" . $platform . "\` in " . $targetdir . " ...\n";   # " 

#Construct the name of the zip file by extracting the last directory name
$zipfiles = $targetdir;
$zipfiles =~ s/.*\/([\w|-]*)$/$1/g;
$zipfiles = $zipfiles . "/*";

$buildmode = "Release";         # Universally, why do you want to package Debug builds anyway?



#
#   WINDOWS builds happen here ...
#
if ($platform =~ m/Windows/  || $platform =~ m/CYGWIN/) {
    
    $platformname = 'Win32';    # Needed this way by nmake
    if (-e "$targetdir.zip") {
        print ("Deleting old target file \'$targetdir.zip\' \n");
        unlink("$targetdir.zip");
    }
    
    # Make the target directory and its main subdirectories
    psystem ("mkdir $targetdir");
    psystem ("mkdir $targetdir/bin");
    psystem ("mkdir $targetdir/lib");
    psystem ("mkdir $targetdir/include");
    psystem ("mkdir $targetdir/samples");
    psystem ("mkdir $targetdir/samples/Projects");
    psystem ("mkdir $targetdir/samples/Projects/Win32");
    psystem ("mkdir $targetdir/samples/data");
    psystem ("mkdir $targetdir/samples/SAXCount");
    psystem ("mkdir $targetdir/samples/SAX2Count");
    psystem ("mkdir $targetdir/samples/SAXPrint");
    psystem ("mkdir $targetdir/samples/SAX2Print");
    psystem ("mkdir $targetdir/samples/DOMCount");
    psystem ("mkdir $targetdir/samples/DOMPrint");
    psystem ("mkdir $targetdir/samples/Redirect");
    psystem ("mkdir $targetdir/samples/MemParse");
    psystem ("mkdir $targetdir/samples/PParse");
    psystem ("mkdir $targetdir/samples/StdInParse");
    psystem ("mkdir $targetdir/samples/EnumVal");
    psystem ("mkdir $targetdir/samples/CreateDOMDocument");
    psystem ("mkdir $targetdir/doc");
    psystem ("mkdir $targetdir/doc/html");
    psystem ("mkdir $targetdir/doc/html/apiDocs");
    
    # If 'FileOnly' NetAccessor has been specified, then the project files have to be
    # changed.
    if ($opt_n =~ m/fileonly/i) {
        changeWindowsProjectForFileOnlyNA("$XERCESCROOT/Projects/Win32/VC6/xerces-all/XercesLib/XercesLib.dsp");
    }
    

    #
    #	ICU Build happens here, if one is required.
    #
    if ($opt_t =~ m/icu/i && length($ICUROOT) > 0) {
        print ("Building ICU from $ICUROOT ...\n");
        
        #Clean up all the dependency files, causes problems for nmake
        pchdir ("$ICUROOT");
        psystem ("del /s /f *.dep *.ncb *.plg *.opt");
        
        # Make the icu dlls
        pchdir ("$ICUROOT/source/allinone");
        if (!$opt_j) {   # Optionally suppress ICU build, to speed up overlong builds while debugging.
	    #For nt we ship both debug and release dlls
	    psystem("msdev allinone.dsw /MAKE \"all - $platformname Release\" /REBUILD /OUT buildlog.txt");
	    psystem("cat buildlog.txt");
	    psystem("msdev allinone.dsw /MAKE \"all - $platformname Debug\" /REBUILD /OUT buildlog.txt");
	    psystem("cat buildlog.txt");
        }
        
        change_windows_project_for_ICU("$XERCESCROOT/Projects/Win32/VC6/xerces-all/XercesLib/XercesLib.dsp");
    }
    

    # Clean up all the dependency files, causes problems for nmake
    # Also clean up all MSVC-generated project files that just cache the IDE state
    pchdir ("$XERCESCROOT");
    psystem ("del /s /f *.dep *.ncb *.plg *.opt");
    
    # Make all files in the Xerces-C system including libraries, samples and tests
    pchdir ("$XERCESCROOT/Projects/Win32/VC6/xerces-all");
    psystem( "msdev xerces-all.dsw /MAKE \"all - $platformname $buildmode\" /REBUILD /OUT buildlog.txt");
    system("cat buildlog.txt");
    
    # Build the debug xerces dll.  Both debug and release DLLs
    #   are in the standard binary distribution of Xerces.
    if ($buildmode ne "Debug") {
        psystem("msdev xerces-all.dsw /MAKE \"XercesLib - $platformname Debug\" /REBUILD /OUT buildlog.txt");
        system("cat buildlog.txt");
    }
    
    # Decide where you want the build copied from
    pchdir ($targetdir);
    $BUILDDIR = $XERCESCROOT . "/Build/Win32/VC6/" . $buildmode;
    print "\nBuild is being copied from \'" . $BUILDDIR . "\'";
    
    # Populate the include output directory
    print ("\n\nCopying headers files ...\n");
    
    @headerDirectories =
     qw'sax
		sax2
        framework
        dom
        internal
        parsers
        util
        util/Compilers  
        util/MsgLoaders
        util/MsgLoaders/ICU
        util/MsgLoaders/InMemory
        util/MsgLoaders/MsgCatalog
        util/MsgLoaders/Win32
        util/Platforms
        util/Platforms/AIX
        util/Platforms/HPUX
        util/Platforms/Linux
        util/Platforms/MacOS
        util/Platforms/OS2
        util/Platforms/OS390
        util/Platforms/PTX
        util/Platforms/Solaris
        util/Platforms/Tandem
        util/Platforms/Win32
        util/Transcoders
        util/Transcoders/ICU
        util/Transcoders/Iconv
        util/Transcoders/Win32
        validators
        validators/DTD';
     
    foreach $dir (@headerDirectories) {
        $inclDir = "include/$dir";
        if (! (-e $inclDir)) {
            psystem("mkdir $inclDir");
        }
        $srcDir = "$XERCESCROOT/src/$dir";
        
        # Weed out directories that have no files to copy, to avoid a bunch of
        # warnings from the cp command in the build output.
        opendir(dir, $srcDir);
        @allfiles = readdir(dir);
        closedir(dir);
        foreach $fileKind ("hpp", "c") {
            $matches = grep(/\.$fileKind$/, @allfiles);
            if ($matches > 0) {
                psystem("cp -f $srcDir/*.$fileKind  $inclDir/");  
            }
        }
    }
    
    
    #
    #  Remove internal implementation headers from the DOM include directory.
    #
    psystem ("rm  $targetdir/include/dom/*Impl.hpp");
    psystem ("rm  $targetdir/include/dom/DS*.hpp");    
    
    
    if ($opt_t =~ m/icu/i && length($ICUROOT) > 0) {
        psystem("cp -Rfv $ICUROOT/include/* $targetdir/include");
    }
    
    #
    # Populate the binary output directory
    #
    print ("\n\nCopying binary outputs ...\n");
    psystem("cp -fv $BUILDDIR/*.dll $targetdir/bin");
    psystem("cp -fv $BUILDDIR/*.exe $targetdir/bin");
    
    if ($opt_t =~ m/icu/i && length($ICUROOT) > 0) {
        # Copy the ICU dlls
        psystem("cp -fv $ICUROOT/bin/$buildmode/icuuc.dll $targetdir/bin");
        psystem("cp -fv $ICUROOT/bin/$buildmode/icudata.dll $targetdir/bin");
        # Copy the ICU libs
        psystem("cp -fv $ICUROOT/lib/$buildmode/icuuc.lib $targetdir/lib");
        psystem("cp -fv $ICUROOT/data/icudata.lib $targetdir/lib");
    }
    psystem("cp -fv $BUILDDIR/xerces-c_*.lib $targetdir/lib");
    if ($buildmode ne "Debug") {
        $DEBUGBUILDDIR = "$XERCESCROOT/Build/Win32/VC6/Debug";
        psystem("cp -fv $DEBUGBUILDDIR/xerces-c_*D.lib $targetdir/lib");
        psystem("cp -fv $DEBUGBUILDDIR/xerces*D.dll $targetdir/bin");
    }
    
    
    # Populate the samples directory
    print ("\n\nCopying sample files ...\n");
    psystem("cp $XERCESCROOT/version.incl $targetdir");
    psystem("cp -Rfv $XERCESCROOT/samples/Projects/* $targetdir/samples/Projects");
    
    psystem("cp -Rfv $XERCESCROOT/samples/SAXCount/* $targetdir/samples/SAXCount");
    psystem("rm -f $targetdir/samples/SAXCount/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/SAX2Count/* $targetdir/samples/SAX2Count");
    psystem("rm -f $targetdir/samples/SAX2Count/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/SAXPrint/* $targetdir/samples/SAXPrint");
    psystem("rm -f $targetdir/samples/SAXPrint/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/SAX2Print/* $targetdir/samples/SAX2Print");
    psystem("rm -f $targetdir/samples/SAX2Print/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/DOMCount/* $targetdir/samples/DOMCount");
    psystem("rm -f $targetdir/samples/DOMCount/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/DOMPrint/* $targetdir/samples/DOMPrint");
    psystem("rm -f $targetdir/samples/DOMPrint/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/Redirect/* $targetdir/samples/Redirect");
    psystem("rm -f $targetdir/samples/Redirect/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/MemParse/* $targetdir/samples/MemParse");
    psystem("rm -f $targetdir/samples/MemParse/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/PParse/* $targetdir/samples/PParse");
    psystem("rm -f $targetdir/samples/PParse/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/StdInParse/* $targetdir/samples/StdInParse");
    psystem("rm -f $targetdir/samples/StdInParse/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/EnumVal/* $targetdir/samples/EnumVal");
    psystem("rm -f $targetdir/samples/EnumVal/Makefile");
    psystem("cp -Rfv $XERCESCROOT/samples/CreateDOMDocument/* $targetdir/samples/CreateDOMDocument");
    psystem("rm -f $targetdir/samples/CreateDOMDocument/Makefile");
    
    psystem("cp -Rfv $XERCESCROOT/samples/data/* $targetdir/samples/data");
    
    # Populate the docs directory
    print ("\n\nCopying documentation ...\n");
    psystem("cp -Rfv $XERCESCROOT/doc/* $targetdir/doc");
    psystem("cp $XERCESCROOT/Readme.html $targetdir");
    psystem("cp $XERCESCROOT/credits.txt $targetdir");   
    psystem("cp $XERCESCROOT/LICENSE.txt $targetdir");

    if (length($ICUROOT) > 0) {
        psystem("cp $XERCESCROOT/license.html $targetdir");
        psystem("cp $XERCESCROOT/license-IBM-public-source.html $targetdir");
    }
    psystem("rm -f $targetdir/doc/Doxyfile");
    psystem("rm -rf $targetdir/doc/dtd");
    psystem("rm -f $targetdir/doc/*.xml");
    psystem("rm -f $targetdir/doc/*.ent");
    psystem("rm -f $targetdir/doc/*.gif");
    
    # Now package it all up using ZIP
    pchdir ("$targetdir/..");
    print ("\n\nZIPping up all files ...\n");
    $zipname = $targetdir . ".zip";
    psystem ("zip -r $zipname $zipfiles");
}
#
#     End of Windows Builds.
 
 
 
#
#  UNIX builds happen here ...
#
if ( ($platform =~ m/AIX/i)    || ($platform =~ m/HP-UX/i) ||
     ($platform =~ m/SunOS/i) || ($platform =~ m/Linux/i) || ($platform =~ m/ptx/i) ) {

    # Set defaults for platform-independent options.
    if ($opt_m eq "") {$opt_m = "inmem";   # In memory  message loader. 
    }
    if ($opt_n eq "") {$opt_n = "socket";  # Socket based net accessor.
    }
    if ($opt_t eq "") {$opt_t = "native";  # Native transcoding service.
    }
    
    
    # Set defaults for platform-specific options.
    if ($platform =~ m/AIX/i) {
        $platform = "aix";
        if ($opt_c eq "") {$opt_c = "xlc_r"; }
        if ($opt_x eq "") {$opt_x = "xlC_r"; }
       
        $icuCompileFlags = 'CXX="xlC_r" ' .
                           'CC="xlc_r" ' .
                           'C_FLAGS="-w -O" ' .
                           'CXX_FLAGS="-w -O" ';
    }
    if ($platform eq 'HP-UX') {
        # Find out the operating system version from 'uname -r'
        open(OSVERSION, "uname -r|");
        $osversion = <OSVERSION>;
        chomp($osversion);
        close (OSVERSION);
        $platform = 'hp-11' if ($osversion =~ m/11\./);
        $platform = 'hp-10' if ($osversion =~ m/10\./);
        
        if ($opt_c eq "") {$opt_c = "cc"; }
        if ($opt_x eq "") {
            $opt_x = "CC";
            if ($platform eq "hp-11") {
                $opt_x = "aCC";
            }
        }
        if ($opt_m eq "") {
            $opt_m = "inmem";
        }
        
        if ($opt_x eq 'CC') {
            $icuCompileFlags = 'CC=cc CXX=CC CXXFLAGS="+eh +DAportable -w -O" CFLAGS="+DAportable -w -O"';
        } else {
            $icuCompileFlags = 'CC=cc CXX=aCC CXXFLAGS="+DAportable -w -O" CFLAGS="+DAportable -w -O"';
        }
        
    }
    if ($platform =~ m/Linux/i) {
        $icuCompileFlags = 'CC=gcc CXX=g++ CXXFLAGS="-w -O" CFLAGS="-w -O"';
        $platform = "linux";
        if ($opt_c eq "") {$opt_c = "gcc";}
        if ($opt_x eq "") {$opt_x = "g++";}
    }
    
    if ($platform =~ m/SunOS/i) {
        $icuCompileFlags = 'CC=cc CXX=CC CXXFLAGS="-w -O" CFLAGS="-w -O"';
        $platform = "solaris";
        if ($opt_c eq "") {$opt_c = "cc";}
        if ($opt_x eq "") {$opt_x = "CC";}
    }
    
    if ($platform =~ m/ptx/i) {
        # Check if the patches have been applied or not
        $platform = "ptx";
        if (!(-d "$XERCESCROOT/src/util/Platforms/PTX")) {
            print ("Error: Could not locate PTX-specific XML4C directory.\n");
            print ("    The PTX-specific patches must be applied to both XML4C and ICU before a build can succeed.\n");
            exit(-1);
        }
        # Generally speaking, ICU must be built, before XML4C can be built, for ptx.
        # If this case causes problems, we can revisit it in the future. Right now,
        # we fail only if ICUROOT is defined but mh-ptx is not present.
        if (length($ICUROOT)) {
            if (!(-e "$ICUROOT/source/config/mh-ptx")) {
                print ("Error: Could not locate PTX-specific ICU files.\n");
                print ("    The PTX-specific patches must be applied to both XML4C and ICU before a build can succeed.\n");
                exit(-1);
            }
        }
        $icuCompileFlags = 'CC=cc CXX=c++ CXXFLAGS="-w -0" CFLAGS="-w -0"';
        # XMLINSTALL is a ptx-port-specific variable used for manipulating where the files are installed.
        if (!length($ENV{'XMLINSTALL'})) {
            print ("XMLINSTALL has not been explicitly defined. Setting it to \'$targetdir\'.\n");
            $ENV{'XMLINSTALL'} = $targetdir;
        }
        $XMLINSTALL = $ENV{'XMLINSTALL'};
    }
    
    # Check if the target directories already exist or not
    if (-e $targetdir.".tar") {
        print ("Error: The target file \'$targetdir.tar\' already exists.\n");
        print ("       You must delete the file \'$targetdir.tar\' to package your product.\n");
        exit(1);
    }
    
    if (-e $srctargetdir.".tar") {
        print ("Error: The target file \'$srctargetdir.tar\' already exists.\n");
        print ("       You must delete the file \'$srctargetdir.tar\' to package your product.\n");
        exit(1);
    }
    
    
    # Make the target directory and its main subdirectories
    psystem ("mkdir $targetdir");
    psystem ("mkdir $targetdir/bin");
    psystem ("mkdir $targetdir/lib");
    psystem ("mkdir $targetdir/include");
    if (length($ICUROOT) > 0) {
        psystem ("mkdir $targetdir/include/unicode");
    }
    psystem ("mkdir $targetdir/include/sax");
	psystem ("mkdir $targetdir/include/sax2");
    psystem ("mkdir $targetdir/include/framework");
    psystem ("mkdir $targetdir/include/internal");
    psystem ("mkdir $targetdir/include/parsers");
    psystem ("mkdir $targetdir/include/util");
    psystem ("mkdir $targetdir/include/util/Compilers");
    psystem ("mkdir $targetdir/include/util/MsgLoaders");
    psystem ("mkdir $targetdir/include/util/MsgLoaders/ICU");
    psystem ("mkdir $targetdir/include/util/MsgLoaders/InMemory");
    psystem ("mkdir $targetdir/include/util/MsgLoaders/MsgCatalog");
    psystem ("mkdir $targetdir/include/util/MsgLoaders/Win32");
    psystem ("mkdir $targetdir/include/util/Platforms");
    psystem ("mkdir $targetdir/include/util/Platforms/AIX");
    psystem ("mkdir $targetdir/include/util/Platforms/HPUX");
    psystem ("mkdir $targetdir/include/util/Platforms/Linux");
    psystem ("mkdir $targetdir/include/util/Platforms/MacOS");
    psystem ("mkdir $targetdir/include/util/Platforms/OS2");
    psystem ("mkdir $targetdir/include/util/Platforms/OS390");
    psystem ("mkdir $targetdir/include/util/Platforms/PTX");
    psystem ("mkdir $targetdir/include/util/Platforms/Solaris");
    psystem ("mkdir $targetdir/include/util/Platforms/Tandem");
    psystem ("mkdir $targetdir/include/util/Platforms/Win32");
    psystem ("mkdir $targetdir/include/util/Transcoders");
    psystem ("mkdir $targetdir/include/util/Transcoders/ICU");
    psystem ("mkdir $targetdir/include/util/Transcoders/Iconv");
    psystem ("mkdir $targetdir/include/util/Transcoders/Win32");
    psystem ("mkdir $targetdir/include/dom");
    psystem ("mkdir $targetdir/include/validators");
    psystem ("mkdir $targetdir/include/validators/DTD");
    psystem ("mkdir $targetdir/samples");
    psystem ("mkdir $targetdir/samples/data");
    psystem ("mkdir $targetdir/samples/SAXCount");
    psystem ("mkdir $targetdir/samples/SAX2Count");
    psystem ("mkdir $targetdir/samples/SAXPrint");
	psystem ("mkdir $targetdir/samples/SAX2Print");
    psystem ("mkdir $targetdir/samples/DOMCount");
    psystem ("mkdir $targetdir/samples/DOMPrint");
    psystem ("mkdir $targetdir/samples/Redirect");
    psystem ("mkdir $targetdir/samples/MemParse");
    psystem ("mkdir $targetdir/samples/PParse");
    psystem ("mkdir $targetdir/samples/StdInParse");
    psystem ("mkdir $targetdir/samples/EnumVal");
    psystem ("mkdir $targetdir/samples/CreateDOMDocument");
    psystem ("mkdir $targetdir/doc");
    psystem ("mkdir $targetdir/doc/html");
    psystem ("mkdir $targetdir/doc/html/apiDocs");
    
    # Build ICU if needed
    if ($opt_t =~ m/icu/i && !$opt_j)
    {
        print("\n\nBuild ICU ...\n");
        if(length($ICUROOT) == 0) {
           print("Error, ICUROOT not set, can not build ICU\n");
           exit(-1);
           }

        # First make the ICU files executable
        pchdir ("$ICUROOT/source");
        psystem ("chmod +x configure config.*");
        psystem ("chmod +x install-sh");
        $ENV{'ICU_DATA'} = "$ICUROOT/data";
        if ($platform =~ m/ptx/i) {
            psystem ("chmod +x runConfigureICU");
            psystem ("runConfigureICU PTX");
        } else {
            psystem ("$icuCompileFlags configure --prefix=$ICUROOT");
        }
        psystem ("gmake clean"); # Clean up the build, may want to comment this line out!
        psystem ("rm -f $ICUROOT/data/*.o"); # gmake clean is not enough
        psystem ("rm -f $ICUROOT/data/*.c"); # same for .c files
        psystem ("gmake");       # This will take a long time!
        psystem ("gmake install"); # Make this separate since this breaks on Solaris
        
        # Please check if the following needs any change in Version 1.4
        # For the antiquated CC compiler under HPUX, we need to invoke
        # gmake one extra time to generate the .cnv files.
        # if ( ($platform =~ m/hp-/i) && ($opt_x eq 'CC') ) {
        #   system ("gmake");
        # }
    }
    
    # For ptx, ICUROOT must now be set to XMLINSTALL for further work.
    if ($platform =~ m/ptx/i) {
        $ENV{'ICUROOT'} = $ENV{'XMLINSTALL'};
    }
    
    # make the source files
    print("\n\nBuild the xerces-c library ...\n");
    pchdir ("$XERCESCROOT/src");
    
    
    psystem ("chmod +x run* con* install-sh");
    
    if (length($opt_r) > 0) {
        psystem ("runConfigure -p$platform -c$opt_c -x$opt_x -m$opt_m -n$opt_n -t$opt_t -r$opt_r");
    } else {
        psystem ("runConfigure -p$platform -c$opt_c -x$opt_x -m$opt_m -n$opt_n -t$opt_t");
    }
    
    psystem ("gmake clean");     # May want to comment this line out to speed up
    psystem ("gmake");
    
    # Move ICU libs into lib dir, so samples will link.  This matches the structure of
    #   the eventual binary packaging, even though we are doing it in the build directory.
    #
    if (length($ICUROOT) > 0) {
	psystem("cp -f $ICUROOT/lib/libicu-uc.* $XERCESCROOT/lib");
	psystem("cp -f $ICUROOT/data/libicudata.* $XERCESCROOT/lib");
    }

    # Now build the samples
    print("\n\nBuild the samples ...\n");
    pchdir ("$XERCESCROOT/samples");
    psystem ("chmod +x run* con* install-sh");
    psystem ("runConfigure -p$platform -c$opt_c -x$opt_x");
    psystem ("gmake clean");     # May want to comment this line out to speed up
    psystem ("gmake");
    
    # Next build the tests
    print("\n\nBuild the tests ...\n");
    pchdir ("$XERCESCROOT/tests");
    psystem ("chmod +x run* con* install-sh");
    psystem ("runConfigure -p$platform -c$opt_c -x$opt_x");
    psystem ("gmake clean");     # May want to comment this line out to speed up
    psystem ("gmake");
    
    pchdir ($targetdir);
    
    # Populate the include output directory
    print ("\n\nCopying headers files ...\n");
    psystem("cp -Rf $XERCESCROOT/src/sax/*.hpp $targetdir/include/sax");
	psystem("cp -Rf $XERCESCROOT/src/sax2/*.hpp $targetdir/include/sax2");
    psystem("cp -Rf $XERCESCROOT/src/framework/*.hpp $targetdir/include/framework");
    psystem("cp -Rf $XERCESCROOT/src/dom/D*.hpp $targetdir/include/dom");
    
    psystem("cp -Rf $XERCESCROOT/version.incl $targetdir");
    
    psystem("rm -f $targetdir/include/dom/*Impl.hpp");
    psystem("rm -f $targetdir/include/dom/DS*.hpp");
    psystem("cp -Rf $XERCESCROOT/src/internal/*.hpp $targetdir/include/internal");
    psystem("cp -Rf $XERCESCROOT/src/internal/*.c $targetdir/include/internal");
    psystem("cp -Rf $XERCESCROOT/src/parsers/*.hpp $targetdir/include/parsers");
    psystem("cp -Rf $XERCESCROOT/src/parsers/*.c $targetdir/include/parsers");
    psystem("cp -Rf $XERCESCROOT/src/util/*.hpp $targetdir/include/util");
    psystem("cp -Rf $XERCESCROOT/src/util/*.c $targetdir/include/util");
    psystem("cp -Rf $XERCESCROOT/src/util/Compilers/*.hpp $targetdir/include/util/Compilers");
    psystem("cp -Rf $XERCESCROOT/src/util/MsgLoaders/*.hpp $targetdir/include/util/MsgLoaders");
    psystem("cp -Rf $XERCESCROOT/src/util/MsgLoaders/ICU/*.hpp $targetdir/include/util/MsgLoaders/ICU");
    psystem("cp -Rf $XERCESCROOT/src/util/MsgLoaders/InMemory/*.hpp $targetdir/include/util/MsgLoaders/InMemory");
    psystem("cp -Rf $XERCESCROOT/src/util/MsgLoaders/MsgCatalog/*.hpp $targetdir/include/util/MsgLoaders/MsgCatalog");
    psystem("cp -Rf $XERCESCROOT/src/util/MsgLoaders/Win32/*.hpp $targetdir/include/util/MsgLoaders/Win32");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/*.hpp $targetdir/include/util/Platforms");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/AIX/*.hpp $targetdir/include/util/Platforms/AIX");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/HPUX/*.hpp $targetdir/include/util/Platforms/HPUX");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/Linux/*.hpp $targetdir/include/util/Platforms/Linux");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/MacOS/*.hpp $targetdir/include/util/Platforms/MacOS");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/OS2/*.hpp $targetdir/include/util/Platforms/OS2");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/OS390/*.hpp $targetdir/include/util/Platforms/OS390");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/PTX/*.hpp $targetdir/include/util/Platforms/PTX");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/Solaris/*.hpp $targetdir/include/util/Platforms/Solaris");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/Tandem/*.hpp $targetdir/include/util/Platforms/Tandem");
    psystem("cp -Rf $XERCESCROOT/src/util/Platforms/Win32/*.hpp $targetdir/include/util/Platforms/Win32");
    psystem("cp -Rf $XERCESCROOT/src/util/Transcoders/*.hpp $targetdir/include/util/Transcoders");
    psystem("cp -Rf $XERCESCROOT/src/util/Transcoders/ICU/*.hpp $targetdir/include/util/Transcoders/ICU");
    psystem("cp -Rf $XERCESCROOT/src/util/Transcoders/Iconv/*.hpp $targetdir/include/util/Transcoders/Iconv");
    psystem("cp -Rf $XERCESCROOT/src/util/Transcoders/Win32/*.hpp $targetdir/include/util/Transcoders/Win32");
    psystem("cp -Rf $XERCESCROOT/src/validators/DTD/*.hpp $targetdir/include/validators/DTD");
    
    if (length($ICUROOT) > 0) {
        print "\nICU files are being copied from \'$ICUROOT\'";
        psystem("cp -Rf $ICUROOT/include/* $targetdir/include");
    }
    
    # Populate the binary output directory
    print ("\n\nCopying binary outputs ...\n");
    psystem("cp -Rf $XERCESCROOT/bin/* $targetdir/bin");
    
    
    
    #if (length($ICUROOT) > 0) {
    #    psystem("cp -f $ICUROOT/lib/libicu-uc.* $targetdir/lib");
    #    psystem("cp -f $ICUROOT/data/libicudata.* $targetdir/lib");
    #}
    
    psystem("cp -f $XERCESCROOT/lib/*.a $targetdir/lib");
    psystem("cp -f $XERCESCROOT/lib/*.so $targetdir/lib");
    psystem("cp -f $XERCESCROOT/lib/*.sl $targetdir/lib");
    
    psystem("rm -rf $targetdir/bin/obj");
    
    # Populate the samples directory
    print ("\n\nCopying sample files ...\n");
    foreach $iii ('config.guess', 'config.h.in', 'config.sub', 'configure', 'configure.in',
                  'install-sh', 'runConfigure', 'Makefile.in', 'Makefile.incl', 'Makefile') {
        psystem("cp -f $XERCESCROOT/samples/$iii $targetdir/samples");
    }
    
    psystem("cp -Rf $XERCESCROOT/samples/data/* $targetdir/samples/data");
    psystem("cp -Rf $XERCESCROOT/samples/SAXCount/* $targetdir/samples/SAXCount");
    psystem("rm -f $targetdir/samples/SAXCount/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/SAX2Count/* $targetdir/samples/SAX2Count");
    psystem("rm -f $targetdir/samples/SAX2Count/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/SAXPrint/* $targetdir/samples/SAXPrint");
    psystem("rm -f $targetdir/samples/SAXPrint/Makefile");
	psystem("cp -Rf $XERCESCROOT/samples/SAX2Print/* $targetdir/samples/SAX2Print");
    psystem("rm -f $targetdir/samples/SAX2Print/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/DOMCount/* $targetdir/samples/DOMCount");
    psystem("rm -f $targetdir/samples/DOMCount/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/DOMPrint/* $targetdir/samples/DOMPrint");
    psystem("rm -f $targetdir/samples/DOMPrint/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/Redirect/* $targetdir/samples/Redirect");
    psystem("rm -f $targetdir/samples/Redirect/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/MemParse/* $targetdir/samples/MemParse");
    psystem("rm -f $targetdir/samples/MemParse/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/PParse/* $targetdir/samples/PParse");
    psystem("rm -f $targetdir/samples/PParse/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/StdInParse/* $targetdir/samples/StdInParse");
    psystem("rm -f $targetdir/samples/StdInParse/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/EnumVal/* $targetdir/samples/EnumVal");
    psystem("rm -f $targetdir/samples/EnumVal/Makefile");
    psystem("cp -Rf $XERCESCROOT/samples/CreateDOMDocument/* $targetdir/samples/CreateDOMDocument");
    psystem("rm -f $targetdir/samples/CreateDOMDocument/Makefile");
    psystem("rm -f $targetdir/samples/Makefile");
    
    # Populate the docs directory
    print ("\n\nCopying documentation ...\n");
    psystem("cp -Rf $XERCESCROOT/doc/* $targetdir/doc");
    psystem("cp $XERCESCROOT/Readme.html $targetdir");
    psystem("cp $XERCESCROOT/credits.txt $targetdir");   
    psystem("cp $XERCESCROOT/LICENSE.txt $targetdir");
    if (length($ICUROOT) > 0) {
        psystem("cp $XERCESCROOT/license.html $targetdir");
        psystem("cp $XERCESCROOT/license-IBM-public-source.html $targetdir");
    }
    psystem("rm -f $targetdir/doc/Doxyfile");
    psystem("rm -rf $targetdir/doc/dtd");
    psystem("rm -f $targetdir/doc/*.xml");
    psystem("rm -f $targetdir/doc/*.ent");
    psystem("rm -f $targetdir/doc/*.gif");
    
    # Change the directory permissions
    psystem ("chmod 644 `find $targetdir -type f`");
    psystem ("chmod 755 $targetdir/bin/* $targetdir/lib/*.sl $targetdir/lib/*.so $targetdir/lib/*.a");
    psystem ("chmod +x $targetdir/samples/runConfigure $targetdir/samples/configure $targetdir/samples/install-sh");
    psystem ("chmod +x $targetdir/samples/config.sub $targetdir/samples/config.guess $targetdir/samples/config.status");
    psystem ("chmod 755 `find $targetdir -type d`");
    
    # Now package it all up using tar
    print ("\n\nTARing up all files ...\n");
    pchdir ("$targetdir/..");
    $zipname = $targetdir . ".tar";
    $platformzipname = $zipname;
    
    psystem ("tar -cvf $platformzipname $zipfiles");
    
    # Finally compress the files
    print ("Compressing $platformzipname ...\n");
    psystem ("gzip $platformzipname");
}
  
#
#  psystem subroutine both prints and executes a system command.
#
sub psystem() {
    print("$_[0]\n");
    system($_[0]);
    }
    
    
#
#  chdir subroutine both prints and executes a chdir
#
sub pchdir() {
    print("chdir $_[0]\n");
    chdir $_[0];
    }
    
    
sub change_windows_project_for_ICU() {
    my ($thefile) = @_;
    print "\nConverting Windows Xerces library project ($thefile) for ICU usage...";
    my $thefiledotbak = $thefile . ".bak";
    rename ($thefile, $thefiledotbak);
    
    open (FIZZLE, $thefiledotbak);
    open (FIZZLEOUT, ">$thefile");
    while ($line = <FIZZLE>) {
        $line =~ s/\/D "PROJ_XMLPARSER"/\/I \"$ICUROOT\\include" \/D "PROJ_XMLPARSER"/g;
        $line =~ s/Debug\/xerces-c_1D.lib"/Debug\/xerces-c_1D.lib" \/libpath:"$ICUROOT\\lib\\Debug" \/libpath:"$ICUROOT\\bin\\Debug"/g;
        $line =~ s/Release\/xerces-c_1.lib"/Release\/xerces-c_1.lib" \/libpath:"$ICUROOT\\lib\\Release" \/libpath:"$ICUROOT\\bin\\Release"/g;
        $line =~ s/XML_USE_WIN32_TRANSCODER/XML_USE_ICU_TRANSCODER/g;
        $line =~ s/user32\.lib/user32\.lib icuuc\.lib icudata\.lib/g;
        $line =~ s/Transcoders\\Win32\\Win32TransService\.cpp/Transcoders\\ICU\\ICUTransService\.cpp/g;
        $line =~ s/Transcoders\\Win32\\Win32TransService\.hpp/Transcoders\\ICU\\ICUTransService\.hpp/g;
        print FIZZLEOUT $line;
    }
    close (FIZZLEOUT);
    close (FIZZLE);
    unlink ($thefiledotbak);
}
  
      
#
# This subroutine is used to munge the XercesLib project file to remove all
# traces of WinSock based NetAccessor. Once no NetAccessor is specified, the
# project file is configured for using the 'FileOnly' NetAccessor.
#
# For this function to work the assumption is that project file in CMVC is
# preconfigured to already use the WinSock based NetAccessor. So, the changes
# that need to be done are:
#   - to remove references to any #defines
#   - to remove references to wsock32.lib
#   - to remove references to the source files for the WinSock based NetAccessor.
#

sub changeWindowsProjectForFileOnlyNA() {
    my ($thefile) = @_;
    print "\nConfiguring Xerces library project ($thefile) for FileOnly NetAccessor...";
    my $thefiledotbak = $thefile . ".bak";
    rename ($thefile, $thefiledotbak);
    
    open (PROJFILEIN, $thefiledotbak);
    open (PROJFILEOUT, ">$thefile");
    
    while ($aline = <PROJFILEIN>) {
        # By skipping over lines between the NetAccessors group
        # we can references to the WinSock based NetAccessor files.
        if ($aline =~ m/^# Begin Group \"NetAccessors\"/g) {
            # ...found it. Write out the line as a place holder. Also...
            print PROJFILEOUT $aline;
            # ...preserve the next two lines.
            $aline = <PROJFILEIN>;
            print PROJFILEOUT $aline;
            $aline = <PROJFILEIN>;
            print PROJFILEOUT $aline;
            # Skip over the lines till you hit the WinSock NetAccessor 'End Group'.
            while ($aline = <PROJFILEIN>) { # read the next line
                last if ($aline =~ m/^# End Group/g);
            }
            # We need to preserve the 'End Group' line. The last statement of the
            # enclosing while loop prints it out.
        }
        
        # From the remaining lines, remove any references to the #defines and
        # the WinSock library.
        $aline =~ s/\/D \"XML_USE_NETACCESSOR_WINSOCK\" //g;  # "
        $aline =~ s/wsock32.lib //g;
        
        print PROJFILEOUT $aline;
    }
    close (PROJFILEOUT);
    close (PROJFILEIN);
    unlink ($thefiledotbak);
}
