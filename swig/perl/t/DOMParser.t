# Before `make install' is performed this script should be runnable
# with `make test'. After `make install' it should work as `perl
# DOMParser.t'

######################### Begin module loading

# use blib;
use Test::More tests => 13;
BEGIN { use_ok("XML::Xerces::DOM") };

use lib 't';
use TestUtils qw($PERSONAL_FILE_NAME);
use vars qw($error);
use strict;

######################### Begin Test

my $document = q[<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<contributors>
  <person Role="manager">
    <name>Mike Pogue</name>
    <email>mpogue@us.ibm.com</email>
  </person>
  <person Role="developer">
    <name>Tom Watson</name>
    <email>rtwatson@us.ibm.com</email>
  </person>
  <person Role="tech writer">
    <name>Susan Hardenbrook</name>
    <email>susanhar@us.ibm.com</email>
  </person>
</contributors>];

my $DOM = XML::Xerces::XercesDOMParser->new();
$DOM->setErrorHandler(XML::Xerces::PerlErrorHandler->new());
$DOM->setValidationScheme ($XML::Xerces::AbstractDOMParser::Val_Always);

eval{$DOM->parse(XML::Xerces::MemBufInputSource->new($document, 'foo'))};
ok($@,'no dtd');

# this is stupid!! We should be able to reset the parser
# after a fatal error - FIXME
TODO: {
  local $TODO = "cannot reuse parser after fatal error";
  $DOM = XML::Xerces::XercesDOMParser->new();
  fail("reset parser");
}

# ensure we can unset the error handler using undef
$DOM->setErrorHandler(undef);
$DOM->setValidationScheme ($XML::Xerces::AbstractDOMParser::Val_Always);
eval{$DOM->parse(XML::Xerces::MemBufInputSource->new($document))};
ok((not $@),'unsetting error handler')
  or XML::Xerces::error($@);

# no validation allows skipping the DTD
$DOM->setValidationScheme ($XML::Xerces::AbstractDOMParser::Val_Never);
$DOM->setErrorHandler(XML::Xerces::PerlErrorHandler->new());

eval{$DOM->parse(XML::Xerces::MemBufInputSource->new($document, 'foo'))};
ok((not $@),'membuf parse');

# test the overloaded parse version
$DOM->parse($PERSONAL_FILE_NAME);
pass('parse filename');


# test the progressive parsing interface
my $token = XML::Xerces::XMLPScanToken->new();
$DOM->parseFirst($PERSONAL_FILE_NAME,$token);
while ($DOM->parseNext($token)) {
  # do nothing
}
pass('progressive parse');



# test that we can reuse the parse again and again
$document = <<\END;
<?xml version="1.0" encoding="iso-8859-1" standalone="no"?>

<!-- @version: -->
<personnel>

  <person id="Big.Boss">
    <name><family>Boss</family> <given>Big</given></name>
    <email>chief@foo.com</email>
    <link subordinates="one.worker two.worker three.worker four.worker five.worker"/>
  </person>

  <foo id="two.worker">
    <name><family>Worker</family> <given>Two</given></name>
    <email>two@foo.com</email>
    <link manager="Big.Boss"/>
  </person>

</personnel>
END
package MyErrorHandler;
use strict;
use vars qw(@ISA);
@ISA = qw(XML::Xerces::PerlErrorHandler);
sub warning {
}

sub error {
  my $LINE = $_[1]->getLineNumber;
  my $COLUMN = $_[1]->getColumnNumber;
  my $MESSAGE = $_[1]->getMessage;
  $::error = <<"EOE";
ERROR:
LINE:    $LINE
COLUMN:  $COLUMN
MESSAGE: $MESSAGE
EOE
  die();
}

sub fatal_error {
  my $LINE = $_[1]->getLineNumber;
  my $COLUMN = $_[1]->getColumnNumber;
  my $MESSAGE = $_[1]->getMessage;
  $::error = <<"EOE";
FATAL ERROR:
LINE:    $LINE
COLUMN:  $COLUMN
MESSAGE: $MESSAGE
EOE
  die();
}
1;

package main;
$token = XML::Xerces::XMLPScanToken->new();
$DOM->setErrorHandler(MyErrorHandler->new());
$::error = '';
eval {
  $DOM->parseFirst(XML::Xerces::MemBufInputSource->new($document),$token);
  while ($DOM->parseNext($token)) {
  }
};
ok($::error,
   'fatal error in progressive parse')
  or XML::Xerces::error($@);

my $doc = $DOM->getDocument();
ok($doc,'doc');
my $root = $doc->getDocumentElement();
ok($root,'root');
my ($person) = $root->getElementsByTagName('person');
ok($person,'person');
my ($foo) = $root->getElementsByTagName('foo');
ok($foo,'foo');

$::error = '';
$DOM->parseReset($token);
eval {
  $DOM->parse(XML::Xerces::MemBufInputSource->new($document));
};
ok($::error,'fatal error in parse')
  or diag("Here's the error: $@");
