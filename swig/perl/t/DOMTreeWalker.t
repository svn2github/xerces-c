# Before `make install' is performed this script should be runnable
# with `make test'. After `make install' it should work as `perl
# DOMTreeWalker.t'

######################### Begin module loading

# use blib;
use Test::More tests => 7;
BEGIN { use_ok("XML::Xerces::DOM") };

use strict;

######################### Begin Test

package MyNodeFilter;
use strict;
use vars qw(@ISA);
@ISA = qw(XML::Xerces::PerlNodeFilter);
sub acceptNode {
  my ($self,$node) = @_;
  return $XML::Xerces::DOMNodeFilter::FILTER_ACCEPT;
}

package main;

# Create a couple of identical test documents
my $document = q[<?xml version="1.0" encoding="UTF-8"?>
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

my $DOM = new XML::Xerces::XercesDOMParser;
my $ERROR_HANDLER = XML::Xerces::PerlErrorHandler->new();
$DOM->setErrorHandler($ERROR_HANDLER);
eval{$DOM->parse(XML::Xerces::MemBufInputSource->new($document))};
XML::Xerces::error($@) if $@;

my $doc = $DOM->getDocument();
isa_ok($doc,'XML::Xerces::DOMDocumentTraversal');
my $root = $doc->getDocumentElement();
my $filter = MyNodeFilter->new();
isa_ok($filter,'XML::Xerces::PerlNodeFilter');
my $what = $XML::Xerces::DOMNodeFilter::SHOW_ELEMENT;
my $walker = eval{$doc->createTreeWalker($root,$what,$filter,1)};
XML::Xerces::error($@) if $@;

isa_ok($walker,'XML::Xerces::DOMTreeWalker');

# test parentNode
$walker->nextNode();
ok($walker->parentNode() == $root,
  "first node is root");

my $success = 1;
my $count = 0;
while (my $node = $walker->nextNode()) {
  $count++;
  $success = 0 unless $node->isa('XML::Xerces::DOMElement');
}
# test that we only got elements
ok($success,
  "TreeWalker returns only DOMElements");

#test that we got all the elements
ok($count == 9,
  "TreeWalker traverses all elements in tree");
