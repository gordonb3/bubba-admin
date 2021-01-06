#!/usr/bin/perl -w
use ExtUtils::MakeMaker;

WriteMakefile( DIR => [ 'web-admin' ] ,	INSTALLDIRS => 'vendor',
	INSTALLVENDORSCRIPT => '/opt/bubba/bin',
	INSTALLSITESCRIPT => '/opt/bubba/bin');
