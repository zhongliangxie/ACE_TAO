// $Id$

/* ===================================================================== */
/*
 * This file is part of CARDAMOM (R) which is jointly developed by THALES
 * and SELEX-SI. It is derivative work based on PERCO Copyright (C) THALES 
 * 2000-2003. All rights reserved.
 * Copyright (C) THALES 2004-2005. All rights reserved.
 * Copyright (C) SELEX-SI and THALES 2006. All rights reserved.
 * 
 * CARDAMOM is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published
 * by the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * CARDAMOM is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public
 * License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with CARDAMOM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/
/* ===================================================================== */
/*

COPYRIGHT

Copyright 1992, 1993, 1994 Sun Microsystems, Inc.  Printed in the United
States of America.  All Rights Reserved.

This product is protected by copyright and distributed under the following
license restricting its use.

The Interface Definition Language Compiler Front End (CFE) is made
available for your use provided that you include this license and copyright
notice on all media and documentation and the software program in which
this product is incorporated in whole or part. You may copy and extend
functionality (but may not remove functionality) of the Interface
Definition Language CFE without charge, but you are not authorized to
license or distribute it to anyone else except as part of a product or
program developed by you or with the express written consent of Sun
Microsystems, Inc. ("Sun").

The names of Sun Microsystems, Inc. and any of its subsidiaries or
affiliates may not be used in advertising or publicity pertaining to
distribution of Interface Definition Language CFE as permitted herein.

This license is effective until terminated by Sun for failure to comply
with this license.  Upon termination, you shall destroy or return all code
and documentation for the Interface Definition Language CFE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED AS IS WITH NO WARRANTIES OF
ANY KIND INCLUDING THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS
FOR A PARTICULAR PURPOSE, NONINFRINGEMENT, OR ARISING FROM A COURSE OF
DEALING, USAGE OR TRADE PRACTICE.

INTERFACE DEFINITION LANGUAGE CFE IS PROVIDED WITH NO SUPPORT AND WITHOUT
ANY OBLIGATION ON THE PART OF Sun OR ANY OF ITS SUBSIDIARIES OR AFFILIATES
TO ASSIST IN ITS USE, CORRECTION, MODIFICATION OR ENHANCEMENT.

SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES SHALL HAVE NO LIABILITY WITH
RESPECT TO THE INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY
INTERFACE DEFINITION LANGUAGE CFE OR ANY PART THEREOF.

IN NO EVENT WILL SUN OR ANY OF ITS SUBSIDIARIES OR AFFILIATES BE LIABLE FOR
ANY LOST REVENUE OR PROFITS OR OTHER SPECIAL, INDIRECT AND CONSEQUENTIAL
DAMAGES, EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

Use, duplication, or disclosure by the government is subject to
restrictions as set forth in subparagraph (c)(1)(ii) of the Rights in
Technical Data and Computer Software clause at DFARS 252.227-7013 and FAR
52.227-19.

Sun, Sun Microsystems and the Sun logo are trademarks or registered
trademarks of Sun Microsystems, Inc.

SunSoft, Inc.
2550 Garcia Avenue
Mountain View, California  94043

NOTE:

SunOS, SunSoft, Sun, Solaris, Sun Microsystems or the Sun logo are
trademarks or registered trademarks of Sun Microsystems, Inc.

 */

#include "CdmwBEGlobal.hpp"

#include "ace/Log_Msg.h"

CDMW_IDL3_TO_IDL2_BE_Export CdmwBEGlobalData *cdmw_be_global = 0;

CdmwBEGlobalData::CdmwBEGlobalData (void)
  : BE_GlobalData (),
    gen_equivalent_idl_ (true),
    gen_internal_idl_ (true),
    gen_cdmw_cif_idl_ (true)
{
  this->gen_copyright_ = false;
}

CdmwBEGlobalData::~CdmwBEGlobalData (void)
{
}

void
CdmwBEGlobalData::parse_args (long &i, char **av)
{
  switch (av[i][1])
  {
    case 'G':
      // Enable generation of ...
      if (av[i][2] == 'e')
      {
        // Equivalent IDL.
        this->gen_equivalent_idl_ = true;
        this->gen_internal_idl_ = false;
        this->gen_cdmw_cif_idl_ = false;
      }
      else if (av[i][2] == 'i')
      {
        // Internal IDL.
        this->gen_equivalent_idl_ = false;
        this->gen_internal_idl_ = true;
        this->gen_cdmw_cif_idl_ = false;
      }
      else if (av[i][2] == 'c')
      {
        // CDM-specific IDL.
        this->gen_equivalent_idl_ = false;
        this->gen_internal_idl_ = false;
        this->gen_cdmw_cif_idl_ = true;
      }
      else if (av[i][2] == 'a')
      {
        // All of the above.
        this->gen_equivalent_idl_ = true;
        this->gen_internal_idl_ = true;
        this->gen_cdmw_cif_idl_ = true;
      }
        
      break;
    default:
      this->BE_GlobalData::parse_args (i, av);
  }
}

void
CdmwBEGlobalData::usage (void) const
{
  ACE_DEBUG ((
      LM_DEBUG,
      ACE_TEXT (" -Ge\t\t\tGenerate only equivalent IDL\n")
    ));
  ACE_DEBUG ((
      LM_DEBUG,
      ACE_TEXT (" -Gi\t\t\tGenerate only CDMW internal IDL\n")
    ));
  ACE_DEBUG ((
      LM_DEBUG,
      ACE_TEXT (" -Gc\t\t\tGenerate only CDMW CIF IDL\n")
    ));
  ACE_DEBUG ((
      LM_DEBUG,
      ACE_TEXT (" -Ga\t\t\tGenerate equivalent, internal, and CIF IDL\n")
    ));
  this->BE_GlobalData::usage ();
}

bool
CdmwBEGlobalData::gen_equivalent_idl (void) const
{
  return this->gen_equivalent_idl_;
}

bool
CdmwBEGlobalData::gen_internal_idl (void) const
{
  return this->gen_internal_idl_;
}

bool
CdmwBEGlobalData::gen_cdmw_cif_idl (void) const
{
  return this->gen_cdmw_cif_idl_;
}

