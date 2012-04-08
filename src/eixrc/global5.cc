// vim:set noet cinoptions= sw=4 ts=4:
// This file is part of the eix project and distributed under the
// terms of the GNU General Public License v2.
//
// Copyright (c)
//   Wolfgang Frisch <xororand@users.sourceforge.net>
//   Emil Beinroth <emilbeinroth@gmx.net>
//   Martin Väth <vaeth@mathematik.uni-wuerzburg.de>

#include <config.h>
#include "global.h"
#include <eixrc/eixrc.h>
#include <eixTk/i18n.h>

#define DEFAULT_PART 5

void fill_defaults_part_5(EixRc &eixrc)
{
#include <eixrc/defaults.cc>
// _( SYSCONFDIR This comment  satisfies check_includes script
}