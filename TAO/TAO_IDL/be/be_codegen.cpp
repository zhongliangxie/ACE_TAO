// $Id$

// ============================================================================
//
// = LIBRARY
//    TAO IDL
//
// = FILENAME
//    be_codegen.cpp
//
// = DESCRIPTION
//   Code generation
//
// = AUTHOR
//    Aniruddha Gokhale
//
// ============================================================================

#include "be_codegen.h"
#include "be_helper.h"
#include "be_visitor_factory.h"
#include "be_stream_factory.h"
#include "be_extern.h"
#include "global_extern.h"
#include "utl_string.h"
#include "idl_defines.h"

ACE_RCSID (be,
           be_codegen,
           "$Id$")

TAO_IDL_BE_Export TAO_CodeGen *tao_cg = 0;

TAO_CodeGen::TAO_CodeGen (void)
  : client_header_ (0),
    client_stubs_ (0),
    client_inline_ (0),
    server_header_ (0),
    implementation_header_(0),
    implementation_skeleton_(0),
    server_template_header_ (0),
    server_skeletons_ (0),
    server_template_skeletons_ (0),
    server_inline_ (0),
    server_template_inline_ (0),
    anyop_source_ (0),
    gperf_input_stream_ (0),
    gperf_input_filename_ (0),
    curr_os_ (0),
    visitor_factory_ (0)
{
  // Initialize the anyop streams here so we won't have to fuss
  // with it in the visitors.
  if (be_global->gen_anyop_files ())
    {
      int status = 0;

      status =
        this->start_anyop_source (
                  be_global->be_get_anyop_source_fname ()
                );

      if (status == -1)
        {
          ACE_ERROR ((LM_ERROR,
                             "(%N:%l) TAO_CodeGen::"
                             "TAO_CodeGen - "
                             "Error opening anyop source file\n"));
        }
    }
}

// destructor
TAO_CodeGen::~TAO_CodeGen (void)
{
  delete this->client_header_;
  delete this->server_header_;
  delete this->implementation_header_;
  delete this->implementation_skeleton_;
  delete this->server_template_header_;
  delete this->client_stubs_;
  delete this->server_skeletons_;
  delete this->server_template_skeletons_;
  delete this->client_inline_;
  delete this->server_inline_;
  delete this->server_template_inline_;
  delete this->anyop_source_;
#if !defined (linux) && !defined (__QNX__)
  // This causes a seg fault on Linux RH 5.1.  Let it leak . . .
  delete this->gperf_input_stream_;
#endif /* ! linux */
  delete [] this->gperf_input_filename_;
  this->curr_os_ = 0;
  //  delete this->visitor_factory_;
}

// visitor factory method
be_visitor *
TAO_CodeGen::make_visitor (be_visitor_context *ctx)
{

  if (this->visitor_factory_ == 0)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                         ACE_TEXT ("TAO_CodeGen::make_visitor - ")
                         ACE_TEXT ("No Visitor Factory\n\n")),
                        0);
    }

  return this->visitor_factory_->make_visitor (ctx);
}

// Change the string to all upper case.
const char *
TAO_CodeGen::upcase (const char *str)
{
  static char upcase_str [NAMEBUFSIZE];

  ACE_OS::memset (upcase_str,
                  '\0',
                  NAMEBUFSIZE);

  // Convert letters in str to upper case.
  for (unsigned int i = 0; i < ACE_OS::strlen (str); ++i)
    {
      if (isalpha (str [i]))
        {
          upcase_str[i] = (char) toupper (str[i]);
        }
      else
        {
          // Copy it as it is.
          upcase_str[i] = str[i];
        }
    }

  return upcase_str;
}

// Set the client header stream.
int
TAO_CodeGen::start_client_header (const char *fname)
{
  if (fname == 0)
    {
      // Bad file name.
      return -1;
    }

  // @@ We are making use of "included_idl_files" that is in the
  // idl_global. We need to make sure the validity of those files.
  idl_global->validate_included_idl_files ();

  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->client_header_ = factory->make_outstream ();

  if (!this->client_header_)
    {
      return -1;
    }

  if (this->client_header_->open (fname, TAO_OutStream::TAO_CLI_HDR) == -1)
    {
      return -1;
    }

  *this->client_header_ << be_nl << "// TAO_IDL - Generated from" << be_nl
                        << "// " << __FILE__ << ":" << __LINE__
                        << be_nl << be_nl;

  // Generate the #ident string, if any.
  this->gen_ident_string (this->client_header_);

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->client_header_,
                           "_TAO_IDL_",
                           "_H_");

  if (be_global->pre_include () != 0)
    {
      *this->client_header_ << "#include /**/ \""
                            << be_global->pre_include ()
                            << "\"\n\n";
    }

  // This one is a good candidate to go first since applications need
  // full knowledge of CORBA::ORB. It's rather heavyweight though
  // (includes PolicyC.h) and should probably be refactored somehow.
  *this->client_header_ << "#include \"tao/ORB.h\"";

  // Some compilers don't optimize the #ifndef header include
  // protection, but do optimize based on #pragma once.
  *this->client_header_ << "\n\n#if !defined (ACE_LACKS_PRAGMA_ONCE)\n"
                        << "# pragma once\n"
                        << "#endif /* ACE_LACKS_PRAGMA_ONCE */";

  *this->client_header_ << be_nl;

  // Other include files.

  if (be_global->stub_export_include () != 0)
    {
      *this->client_header_ << "\n#include \""
                            << be_global->stub_export_include ()
                            << "\"";
    }

  this->gen_stub_hdr_includes ();

  size_t nfiles = idl_global->n_included_idl_files ();

  if (nfiles > 0)
    {
      *this->client_header_ << "\n";
    }

  // We must include all the client headers corresponding to
  // IDL files included by the current IDL file.
  // We will use the included IDL file names as they appeared
  // in the original main IDL file, not the one  which went
  // thru CC preprocessor.
  for (size_t j = 0; j < nfiles; ++j)
    {
      char* idl_name = idl_global->included_idl_files ()[j];

      // Make a String out of it.
      UTL_String idl_name_str = idl_name;

      // Make sure this file was actually got included, not
      // ignored by some #if defined compiler directive.


      // Get the clnt header from the IDL file name.
      const char* client_hdr =
        BE_GlobalData::be_get_client_hdr (&idl_name_str,
                                          1);

      // Sanity check and then print.
      if (client_hdr != 0)
        {
          this->client_header_->print ("\n#include \"%s\"",
                                       client_hdr);
        }
      else
        {
          ACE_ERROR ((LM_WARNING,
                      ACE_TEXT ("\nWARNING, invalid file '%s' included"),
                      idl_name));
        }
    }

  // Generate the TAO_EXPORT_MACRO macro.
  *this->client_header_ << "\n\n#if defined (TAO_EXPORT_MACRO)\n";
  *this->client_header_ << "#undef TAO_EXPORT_MACRO\n";
  *this->client_header_ << "#endif\n";
  *this->client_header_ << "#define TAO_EXPORT_MACRO "
                        << be_global->stub_export_macro ();

  // Generate export macro for nested classes.
  *this->client_header_ << "\n\n#if defined (TAO_EXPORT_NESTED_CLASSES)\n"
                        << "#  if defined (TAO_EXPORT_NESTED_MACRO)\n"
                        << "#    undef TAO_EXPORT_NESTED_MACRO\n"
                        << "#  endif /* defined "
                        << "(TAO_EXPORT_NESTED_MACRO) */\n"
                        << "#  define TAO_EXPORT_NESTED_MACRO "
                        << be_global->stub_export_macro ()
                        << be_nl
                        << "#endif /* TAO_EXPORT_NESTED_CLASSES */";

  *this->client_header_ << "\n\n#if defined(_MSC_VER)\n"
                        << "#if (_MSC_VER >= 1200)\n"
                        << "#pragma warning(push)\n"
                        << "#endif /* _MSC_VER >= 1200 */\n"
                        << "#pragma warning(disable:4250)";

  if (be_global->use_raw_throw ())
    {
      *this->client_header_ << "\n#pragma warning(disable:4290)";
    }

  *this->client_header_ << "\n#endif /* _MSC_VER */";

  *this->client_header_
      << "\n\n#if defined (__BORLANDC__)\n"
      << "#pragma option push -w-rvl -w-rch -w-ccc -w-inl\n"
      << "#endif /* __BORLANDC__ */";

  return 0;
}

// Get the client header stream.
TAO_OutStream *
TAO_CodeGen::client_header (void)
{
  return this->client_header_;
}

// Set the client stub stream.
int
TAO_CodeGen::start_client_stubs (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->client_stubs_ = factory->make_outstream ();

  if (!this->client_stubs_)
    {
      return -1;
    }

  if (this->client_stubs_->open (fname, TAO_OutStream::TAO_CLI_IMPL) == -1)
    {
      return -1;
    }

  *this->client_stubs_ << be_nl << "// TAO_IDL - Generated from" << be_nl
                       << "// " << __FILE__ << ":" << __LINE__
                       << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->client_stubs_);

  this->gen_stub_src_includes ();

  *this->client_stubs_ << "\n\n#if defined (__BORLANDC__)\n"
                       << "#pragma option -w-rvl -w-rch -w-ccc -w-aus -w-sig\n"
                       << "#endif /* __BORLANDC__ */";

  // Generate the code that includes the inline file if not included in the
  // header file.
  *this->client_stubs_ << "\n\n#if !defined (__ACE_INLINE__)";
  *this->client_stubs_ << "\n#include \""
                       << be_global->be_get_client_inline_fname (1)
                       << "\"";
  *this->client_stubs_ << "\n#endif /* !defined INLINE */";

  return 0;
}

// Get the client stubs stream.
TAO_OutStream *
TAO_CodeGen::client_stubs (void)
{
  return this->client_stubs_;
}

// Set the client inline stream.
int
TAO_CodeGen::start_client_inline (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->client_inline_ = factory->make_outstream ();

  if (!this->client_inline_)
    {
      return -1;
    }

  if (this->client_inline_->open (fname, TAO_OutStream::TAO_CLI_INL) == -1)
    {
      return -1;
    }

  // Generate the ident string, if any.
  this->gen_ident_string (this->client_inline_);

  return 0;
}

// Get the client inline stream.
TAO_OutStream *
TAO_CodeGen::client_inline (void)
{
  return this->client_inline_;
}

// Set the server header stream.
int
TAO_CodeGen::start_server_header (const char *fname)
{
  // @@ We are making use of "included_idl_files" that is in the
  // idl_global. We need to make sure the validity of those files.
  idl_global->validate_included_idl_files ();

  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_header_ = factory->make_outstream ();

  if (!this->server_header_)
    {
      return -1;
    }

  if (this->server_header_->open (fname, TAO_OutStream::TAO_SVR_HDR) == -1)
    {
      return -1;
    }

  *this->server_header_ << be_nl << "// TAO_IDL - Generated from" << be_nl
                        << "// " << __FILE__ << ":" << __LINE__
                        << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_header_);

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->server_header_,
                           "_TAO_IDL_",
                           "_H_");

  if (be_global->pre_include () != 0)
    {
      *this->server_header_ << "#include /**/ \""
                            << be_global->pre_include ()
                            << "\"\n";
    }

  // We must include all the skeleton headers corresponding to
  // IDL files included by the current IDL file.
  // We will use the included IDL file names as they appeared
  // in the original main IDL file, not the one  which went
  // thru CC preprocessor.
  for (size_t j = 0;
       j < idl_global->n_included_idl_files ();
       ++j)
    {
      char* idl_name = idl_global->included_idl_files ()[j];

      // String'ifying the name.
      UTL_String idl_name_str (idl_name);

      const char* server_hdr =
        BE_GlobalData::be_get_server_hdr (&idl_name_str, 1);

      this->server_header_->print ("\n#include \"%s\"",
                                   server_hdr);
    }

  // Include the Messaging files if AMI is enabled.
  if (be_global->ami_call_back () == I_TRUE)
    {
      // Include Messaging skeleton file.
      this->gen_standard_include (this->server_header_,
                                  "tao/Messaging/MessagingS.h");
    }

  // The server header should include the client header.
  *this->server_header_ << "\n#include \""
                        << be_global->be_get_client_hdr_fname (1)
                        << "\"";

  // Some compilers don't optimize the #ifndef header include
  // protection, but do optimize based on #pragma once.
  *this->server_header_ << "\n\n#if !defined (ACE_LACKS_PRAGMA_ONCE)\n"
                        << "# pragma once\n"
                        << "#endif /* ACE_LACKS_PRAGMA_ONCE */\n";

  // Include the definitions for the PortableServer namespace,
  // this forces the application to link the POA library, a good
  // thing, because we need the definitions there, it also
  // registers the POA factory with the Service_Configurator, so
  // the ORB can automatically find it.
  this->gen_standard_include (this->server_header_,
                              "tao/Collocation_Proxy_Broker.h");
  this->gen_standard_include (this->server_header_,
                              "tao/PortableServer/PortableServer.h");
  this->gen_standard_include (this->server_header_,
                              "tao/PortableServer/Servant_Base.h");

  if (be_global->gen_amh_classes ())
    {
      this->gen_standard_include (this->server_header_,
                                  "tao/PortableServer/AMH_Response_Handler.h");
    }

  *this->server_header_ << be_nl << be_nl
                        << "#if defined(_MSC_VER)\n"
                        << "#if (_MSC_VER >= 1200)\n"
                        << "#pragma warning(push)\n"
                        << "#endif /* _MSC_VER >= 1200 */\n"
                        << "#pragma warning(disable:4250)";

  if (be_global->use_raw_throw ())
    {
      *this->server_header_ << "\n#pragma warning(disable:4290)";
    }

  *this->server_header_ << "\n#endif /* _MSC_VER */";

  *this->server_header_
      << "\n\n#if defined (__BORLANDC__)\n"
      << "#pragma option push -w-rvl -w-rch -w-ccc -w-inl\n"
      << "#endif /* __BORLANDC__ */";

  if (be_global->skel_export_include () != 0)
    {
      *this->server_header_ << "\n\n#include \""
                            << be_global->skel_export_include ()
                            << "\"";

      // Generate the TAO_EXPORT_MACRO macro.
      *this->server_header_ << "\n\n#if defined (TAO_EXPORT_MACRO)\n";
      *this->server_header_ << "#undef TAO_EXPORT_MACRO\n";
      *this->server_header_ << "#endif\n";
      *this->server_header_ << "#define TAO_EXPORT_MACRO "
                            << be_global->skel_export_macro ();

      // Generate export macro for nested classes.
      *this->server_header_
        << "\n#if defined (TAO_EXPORT_NESTED_CLASSES)\n"
        << "#  if defined (TAO_EXPORT_NESTED_MACRO)\n"
        << "#    undef TAO_EXPORT_NESTED_MACRO\n"
        << "#  endif /* defined (TAO_EXPORT_NESTED_MACRO) */\n"
        << "#  define TAO_EXPORT_NESTED_MACRO "
        << be_global->skel_export_macro ()
        << be_nl
        << "#endif /* TAO_EXPORT_NESTED_CLASSES */";
    }

  return 0;
}

// Get the server header stream.
TAO_OutStream *
TAO_CodeGen::server_header (void)
{
  return this->server_header_;
}

// Set the server header stream.
int
TAO_CodeGen::start_server_template_header (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_template_header_ = factory->make_outstream ();

  if (!this->server_template_header_)
    {
      return -1;
    }

  if (this->server_template_header_->open (fname,
                                           TAO_OutStream::TAO_SVR_TMPL_HDR)
        == -1)
    {
      return -1;
    }

  *this->server_template_header_ << be_nl << "// TAO_IDL - Generated from"
                                 << be_nl
                                 << "// " << __FILE__ << ":" << __LINE__
                                 << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_template_header_);

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->server_template_header_,
                           "_TAO_IDL_",
                           "_H_");

  if (be_global->pre_include () != 0)
    {
      *this->server_template_header_ << "#include /**/ \""
                                     << be_global->pre_include ()
                                     << "\"";
    }

  *this->server_template_header_ << "\n\n#if defined(_MSC_VER)\n"
                                 << "#if (_MSC_VER >= 1200)\n"
                                 << "#pragma warning(push)\n"
                                 << "#endif /* _MSC_VER >= 1200 */\n"
                                 << "#pragma warning(disable:4250)\n";

  if (be_global->use_raw_throw ())
    {
      *this->server_template_header_ << "#pragma warning(disable:4290)\n";
    }

  *this->server_template_header_ << "#endif /* _MSC_VER */\n";

  return 0;
}

// Get the server header stream.
TAO_OutStream *
TAO_CodeGen::server_template_header (void)
{
  return this->server_template_header_;
}

// Set the server skeletons stream.
int
TAO_CodeGen::start_server_skeletons (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_skeletons_ = factory->make_outstream ();

  if (!this->server_skeletons_)
    {
      return -1;
    }

  if (this->server_skeletons_->open (fname,
                                     TAO_OutStream::TAO_SVR_IMPL)
        == -1)
    {
      return -1;
    }

  *this->server_skeletons_ << be_nl << be_nl << "// TAO_IDL - Generated from "
                           << be_nl << "// " << __FILE__ << ":" << __LINE__
                           << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_skeletons_);

  // Generate the include statement for the precompiled header file.
  if (be_global->pch_include ())
    {
      *this->server_skeletons_ << "#include \""
                               << be_global->pch_include ()
                               << "\"\n";
    }

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->server_skeletons_,
                           "_TAO_IDL_",
                           "_CPP_");

  // Generate the include statement for the server header.
  *this->server_skeletons_
    << "\n#include \""
    << be_global->be_get_server_hdr_fname (1)
    << "\"";

  this->gen_skel_src_includes ();

  *this->server_skeletons_ << "\n\n#if defined (__BORLANDC__)\n"
                           << "#pragma option -w-rvl -w-rch -w-ccc -w-aus\n"
                           << "#endif /* __BORLANDC__ */";

   // Generate the code that includes the inline file if not included in the
  // header file.
  *this->server_skeletons_ << "\n\n#if !defined (__ACE_INLINE__)\n";
  *this->server_skeletons_ << "#include \""
                           << be_global->be_get_server_inline_fname (1)
                           << "\"\n";
  *this->server_skeletons_ << "#endif /* !defined INLINE */";

  return 0;
}

// Get the server skeletons stream.
TAO_OutStream *
TAO_CodeGen::server_skeletons (void)
{
  return this->server_skeletons_;
}

// Start the server template skeleton stream.
int
TAO_CodeGen::start_server_template_skeletons (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_template_skeletons_ = factory->make_outstream ();

  if (!this->server_template_skeletons_)
    {
      return -1;
    }

  if (this->server_template_skeletons_->open (fname,
                                              TAO_OutStream::TAO_SVR_TMPL_IMPL)
        == -1)
    {
      return -1;
    }

  *this->server_template_skeletons_ << be_nl << "// TAO_IDL - Generated from "
                                    << be_nl << "// "
                                    << __FILE__ << ":" << __LINE__
                                    << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_template_skeletons_);

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->server_template_skeletons_,
                           "_TAO_IDL_",
                           "_CPP_");

  // Generate the include statement for the server header.
  *this->server_template_skeletons_
      << "#include \""
      << be_global->be_get_server_template_hdr_fname (1)
      << "\"";

  // Generate the code that includes the inline file if not included in the
  // header file.
  *this->server_template_skeletons_ << "\n\n#if !defined (__ACE_INLINE__)";
  *this->server_template_skeletons_
      << "\n#include \""
      << be_global->be_get_server_template_inline_fname (1)
      << "\"";
  *this->server_template_skeletons_ << "\n#endif /* !defined INLINE */\n\n";

  return 0;
}

// Get the server template skeletons stream.
TAO_OutStream *
TAO_CodeGen::server_template_skeletons (void)
{
  return this->server_template_skeletons_;
}

// Set the server inline stream.
int
TAO_CodeGen::start_server_inline (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_inline_ = factory->make_outstream ();

  if (!this->server_inline_)
    {
      return -1;
    }

  if (this->server_inline_->open (fname, TAO_OutStream::TAO_SVR_INL) == -1)
    {
      return -1;
    }

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_inline_);

  return 0;
}

// Get the server inline stream.
TAO_OutStream *
TAO_CodeGen::server_inline (void)
{
  return this->server_inline_;
}

// Set the server template inline stream.
int
TAO_CodeGen::start_server_template_inline (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->server_template_inline_ = factory->make_outstream ();

  if (!this->server_template_inline_)
    {
      return -1;
    }

  if (this->server_template_inline_->open (fname,
                                           TAO_OutStream::TAO_SVR_INL)
       == -1)
    {
      return -1;
    }

  // Generate the ident string, if any.
  this->gen_ident_string (this->server_template_inline_);

  return 0;
}

// Get the server template inline stream.
TAO_OutStream *
TAO_CodeGen::server_template_inline (void)
{
  return this->server_template_inline_;
}

int
TAO_CodeGen::start_anyop_source (const char *fname)
{
  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->anyop_source_ = factory->make_outstream ();

  if (!this->anyop_source_)
    {
      return -1;
    }

  if (this->anyop_source_->open (fname,
                                 TAO_OutStream::TAO_CLI_IMPL)
       == -1)
    {
      return -1;
    }

  // Generate the include statement for the precompiled header file.
  if (be_global->pch_include ())
    {
      *this->anyop_source_ << "#include \""
                           << be_global->pch_include ()
                           << "\"";
    }

  // Generate the include statement for the client header. We just
  // need to put only the base names. Path info is not required.
  *this->anyop_source_ << "\n#include \""
                       << be_global->be_get_client_hdr_fname (1)
                       << "\"";

  this->gen_standard_include (this->anyop_source_,
                              "tao/Typecode.h");

  return 0;
}

TAO_OutStream *
TAO_CodeGen::anyop_source (void)
{
  return this->anyop_source_;
}

// Set the server header stream.
int
TAO_CodeGen::start_implementation_header (const char *fname)
{
  // @@ We are making use of "included_idl_files" that is in the
  // idl_global. We need to make sure the validity of those files.


  idl_global->validate_included_idl_files ();

  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->implementation_header_ = factory->make_outstream ();

  if (!this->implementation_header_)
    {
      return -1;
    }

  if (this->implementation_header_->open (fname,
                                          TAO_OutStream::TAO_IMPL_HDR)
        == -1)
    {
      return -1;
    }

  *this->implementation_header_ << be_nl << "// TAO_IDL - Generated from "
                                << be_nl << "// "
                                << __FILE__ << ":" << __LINE__
                                << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->implementation_header_);

  // Generate the #ifndef clause.
  this->gen_ifndef_string (fname,
                           this->implementation_header_,
                           "",
                           "_H_");

  const char* server_hdr = BE_GlobalData::be_get_server_hdr_fname (1);

  *this->implementation_header_<< "#include \"" << server_hdr <<"\"";

  *this->implementation_header_
    << "\n\n#if !defined (ACE_LACKS_PRAGMA_ONCE)\n"
    << "#pragma once\n"
    << "#endif /* ACE_LACKS_PRAGMA_ONCE */\n\n";

  return 0;
}


// Get the implementation header stream.
TAO_OutStream *
TAO_CodeGen::implementation_header (void)
{
  return this->implementation_header_;
}


// Set the implementation skeleton stream.
int
TAO_CodeGen::start_implementation_skeleton (const char *fname)
{
  // @@ We are making use of "included_idl_files" that is in the
  // idl_global. We need to make sure the validity of those files.
  idl_global->validate_included_idl_files ();

  // Retrieve the singleton instance to the outstream factory.
  TAO_OutStream_Factory *factory = TAO_OUTSTREAM_FACTORY::instance ();

  // Retrieve a specialized instance.
  this->implementation_skeleton_ = factory->make_outstream ();

  if (!this->implementation_skeleton_)
    {
      return -1;
    }

  if (this->implementation_skeleton_->open (fname,
                                            TAO_OutStream::TAO_IMPL_SKEL)
        == -1)
    {
      return -1;
    }

  *this->implementation_skeleton_ << be_nl << "// TAO_IDL - Generated from "
                                  << be_nl << "// "
                                  << __FILE__ << ":" << __LINE__
                                  << be_nl << be_nl;

  // Generate the ident string, if any.
  this->gen_ident_string (this->implementation_skeleton_);

  const char* impl_hdr =
    BE_GlobalData::be_get_implementation_hdr_fname ();

  this->implementation_skeleton_->print ("#include \"%s\"\n\n",
                                         impl_hdr);

  return 0;
}


// Get the implementation header stream.
TAO_OutStream *
TAO_CodeGen::implementation_skeleton (void)
{
  return this->implementation_skeleton_;
}

// Put the last #endif in the client and server headers.
int
TAO_CodeGen::end_client_header (void)
{
  // Generate the <<= and >>= operators here.

  *this->client_header_ << be_nl << be_nl << "// TAO_IDL - Generated from"
                        << be_nl << "// " << __FILE__ << ":" << __LINE__;

  // Insert the code to include the inline file.
  *this->client_header_ << "\n\n#if defined (__ACE_INLINE__)\n";
  *this->client_header_ << "#include \""
                        << be_global->be_get_client_inline_fname (1)
                        << "\"\n";
  *this->client_header_ << "#endif /* defined INLINE */";

  *this->client_header_ << "\n\n#if defined(_MSC_VER) && (_MSC_VER >= 1200)\n"
                        << "#pragma warning(pop)\n"
                        << "#endif /* _MSC_VER */";

  *this->client_header_ << "\n\n#if defined (__BORLANDC__)\n"
                        << "#pragma option pop\n"
                        << "#endif /* __BORLANDC__ */";

  // Code to put the last #endif.
  *this->client_header_ << "\n\n";

  if (be_global->post_include () != 0)
    {
      *this->client_header_ << "#include /**/ \""
                            << be_global->post_include ()
                            << "\"\n\n";
    }

  *this->client_header_ << "#endif /* ifndef */" << be_nl << be_nl;

  return 0;
}

int
TAO_CodeGen::end_server_header (void)
{
  *this->server_header_ << be_nl << be_nl << "// TAO_IDL - Generated from "
                        << be_nl << "// " << __FILE__ << ":" << __LINE__
                        << be_nl << be_nl;

  // Insert the template header.
  if (be_global->gen_tie_classes ())
    {
      *this->server_header_ << "#include \""
                            << be_global->be_get_server_template_hdr_fname (1)
                            << "\"\n";
    }

  // Insert the code to include the inline file.
  *this->server_header_ << "\n#if defined (__ACE_INLINE__)\n";
  *this->server_header_ << "#include \""
                        << be_global->be_get_server_inline_fname (1)
                        << "\"\n";
  *this->server_header_ << "#endif /* defined INLINE */";

  *this->server_header_ << "\n\n#if defined(_MSC_VER) && (_MSC_VER >= 1200)\n"
                        << "#pragma warning(pop)\n"
                        << "#endif /* _MSC_VER */";

  *this->server_header_ << "\n\n#if defined (__BORLANDC__)\n"
                        << "#pragma option pop\n"
                        << "#endif /* __BORLANDC__ */";

  // Code to put the last #endif.
  *this->server_header_ << "\n\n";

  if (be_global->post_include () != 0)
    {
      *this->server_header_ << "#include /**/ \""
                            << be_global->post_include ()
                            << "\"\n";
    }

  *this->server_header_ << "#endif /* ifndef */\n";
  return 0;
}

int
TAO_CodeGen::end_implementation_header (const char *fname)
{
  static char macro_name [NAMEBUFSIZE];

  ACE_OS::memset (macro_name,
                  '\0',
                  NAMEBUFSIZE);

  const char *suffix = ACE_OS::strrchr (fname, '.');

  if (suffix == 0)
    {
      // File seems to have no extension, so let us take the name
      // as it is.
      if (fname == 0)
        {
          // Bad file name.
          return -1;
        }
      else
        {
          suffix = fname;
        }
    }

  // Convert letters in fname to upper case.
  for (int i = 0; i < (suffix - fname); ++i)
    {
      if (isalpha (fname [i]))
        {
          macro_name[i] = (char) toupper (fname [i]);
        }
      else if (isdigit (fname [i]))
        {
          macro_name[i] = fname[i];
        }
      else
        {
          macro_name[i] = '_';
        }
    }

  ACE_OS::strcat (macro_name,
                  "_H_");

  // Code to put the last #endif.
  this->implementation_header_->print ("\n#endif /* %s  */\n",
                                       macro_name);
  return 0;
}

int
TAO_CodeGen::end_server_template_header (void)
{
  *this->server_template_header_ << be_nl << be_nl << "// TAO_IDL - Generated from "
                                 << be_nl << "// "
                                 << __FILE__ << ":" << __LINE__;

  // Insert the code to include the inline file.
  *this->server_template_header_ << "\n\n#if defined (__ACE_INLINE__)";
  *this->server_template_header_
      << "\n#include \""
      << be_global->be_get_server_template_inline_fname (1)
      << "\"";
  *this->server_template_header_ << "\n#endif /* defined INLINE */";

  // Insert the code to include the template source file.
  *this->server_template_header_
      << "\n\n#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)";
  *this->server_template_header_
      << "\n#include \""
      << be_global->be_get_server_template_skeleton_fname (1)
      << "\"";
  *this->server_template_header_ << "\n#endif /* defined REQUIRED SOURCE */";

  // Insert the code to include the template pragma.
  *this->server_template_header_
      << "\n\n#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)";
  *this->server_template_header_
      << "\n#pragma implementation (\""
      << be_global->be_get_server_template_skeleton_fname (1)
      << "\")";
  *this->server_template_header_ << "\n#endif /* defined REQUIRED PRAGMA */";

  *this->server_template_header_ << "\n\n#if defined(_MSC_VER) && (_MSC_VER >= 1200)\n"
                                 << "#pragma warning(pop)\n"
                                 << "#endif /* _MSC_VER */";

  // Code to put the last #endif.
  *this->server_template_header_ << "\n\n";

  if (be_global->post_include () != 0)
    {
      *this->server_template_header_ << "#include /**/ \""
                                     << be_global->post_include ()
                                     << "\"\n";
    }

  *this->server_template_header_ << "#endif /* ifndef */\n";
  return 0;
}

int
TAO_CodeGen::end_server_template_inline (void)
{
  *this->server_template_inline_ << "\n\n";

  return 0;
}

int
TAO_CodeGen::end_server_template_skeletons (void)
{
  // Code to put the last #endif.
  *this->server_template_skeletons_ << "\n#endif /* ifndef */\n";

  return 0;
}

int
TAO_CodeGen::end_server_skeletons (void)
{
  // Code to put the last #endif.
  *this->server_skeletons_ << "\n\n#endif /* ifndef */\n";

  return 0;
}

int
TAO_CodeGen::end_anyop_source (void)
{
  *this->anyop_source_ << "\n\n";

  return 0;
}

// We use the following helper functions to pass information. This class is the
// best place to pass such information rather than passing information through
// global variables spread everywhere. This class is a singleton and is
// effectively a global.

void
TAO_CodeGen::gperf_input_stream (TAO_OutStream *os)
{
#if !defined (linux) && !defined (__QNX__)
  // This causes a seg fault on Linux RH 5.1.  Let it leak . . .
  delete this->gperf_input_stream_;
#endif /* ! linux */
  this->gperf_input_stream_ = os;
}

TAO_OutStream *
TAO_CodeGen::gperf_input_stream (void)
{
  return this->gperf_input_stream_;
}

void
TAO_CodeGen::gperf_input_filename (char *filename)
{
  delete [] this->gperf_input_filename_;
  this->gperf_input_filename_ = filename;
}

char *
TAO_CodeGen::gperf_input_filename (void)
{
  return this->gperf_input_filename_;
}

void
TAO_CodeGen::outstream (TAO_OutStream *os)
{
  this->curr_os_ = os;
}

TAO_OutStream *
TAO_CodeGen::outstream (void)
{
  return this->curr_os_;
}

void
TAO_CodeGen::node (be_decl *n)
{
  this->node_ = n;
}

be_decl *
TAO_CodeGen::node (void)
{
  return this->node_;
}

void
TAO_CodeGen::config_visitor_factory (void)
{
  this->visitor_factory_ = TAO_VISITOR_FACTORY::instance ();
}

void
TAO_CodeGen::gen_ident_string (TAO_OutStream *stream) const
{
  const char *str = idl_global->ident_string ();

  if (str != 0)
    {
      *stream << "#" << str << be_nl << be_nl;
    }
}

void
TAO_CodeGen::gen_ifndef_string (const char *fname,
                                TAO_OutStream *stream,
                                const char *prefix,
                                const char *suffix)
{
  static char macro_name [NAMEBUFSIZE];

  ACE_OS::memset (macro_name,
                  '\0',
                  NAMEBUFSIZE);

  const char *extension = ACE_OS::strrchr (fname, '.');

  if (extension == 0)
    {
      // File seems to have no extension, so let us take the name
      // as it is.
      extension = fname;
    }

  ACE_OS::sprintf (macro_name, prefix);

  size_t offset = ACE_OS::strlen (prefix);

  // Convert letters in fname to upper case.
  for (int i = 0; i < (extension - fname); i++)
    {
      if (isalpha (fname [i]))
        {
          macro_name[i + offset] = (char) toupper (fname [i]);
        }
      else if (isdigit (fname [i]))
        {
          macro_name[i + offset] = fname[i];
        }
      else
        {
          macro_name[i + offset] = '_';
        }
    }

  ACE_OS::strcat (macro_name, suffix);

  // Generate the #ifndef ... #define statements.
  stream->print ("#ifndef %s\n",
                 macro_name);
  stream->print ("#define %s\n\n",
                 macro_name);
}

void
TAO_CodeGen::gen_standard_include (TAO_OutStream *stream,
                                   const char *included_file)
{
  // Switch between changing or non-changing standard include files
  // include files, so that #include statements can be
  // generated with ""s or <>s respectively, for the standard include
  // files (e.g. tao/corba.h).

  const char *start_delimiter = "\"";
  const char *end_delimiter = "\"";

  if (be_global->changing_standard_include_files () == 0)
    {
      start_delimiter = "<";
      end_delimiter = ">";
    }

  *stream << "\n#include " << start_delimiter
          << included_file
          << end_delimiter;
}

void
TAO_CodeGen::gen_stub_hdr_includes (void)
{
  // Always included.

  this->gen_standard_include (this->client_header_,
                              "tao/CDR.h");
  this->gen_standard_include (this->client_header_,
                              "tao/Environment.h");

  // Conditionally included.

  // Non-abstract interface or keyword 'Object'.
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.non_local_iface_seen_
      | idl_global->decls_seen_masks.local_iface_seen_
      | idl_global->decls_seen_masks.base_object_seen_,
      "tao/Object.h",
      this->client_header_
    );

  // Not needed at the moment, since Exception.h is pulled in by ORB.h,
  // included at the top of the stub header file. May change if ORB.h
  // is rearranged to make a lighter include for applications.
#if 0
  // System exception throw spec for every operation may change soon.
  // For IDL exception, we need full knowledge of CORBA::UserException.
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.operation_seen_
      | idl_global->decls_seen_masks.exception_seen_,
      "tao/Exception.h",
      this->client_header_
    );
#endif

  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.abstract_iface_seen_))
    {
      // Include the AbstractBase file from the Valuetype library.
      this->gen_standard_include (this->client_header_,
                                  "tao/Valuetype/AbstractBase.h");

      // Turn on generation of the rest of the Valuetype library includes.
      ACE_SET_BITS (idl_global->decls_seen_info_,
                    idl_global->decls_seen_masks.valuetype_seen_);
    }

  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.valuetype_seen_))
    {
      // Include files from the Valuetype library.
      this->gen_standard_include (this->client_header_,
                                  "tao/Valuetype/ValueBase.h");
      this->gen_standard_include (this->client_header_,
                                  "tao/Valuetype/Valuetype_Adapter_Impl.h");

      // Check for setting this bit performed in y.tab.cpp, actual checking
      // code is in be_valuetype.cpp.
      this->gen_cond_file_include (
          idl_global->decls_seen_masks.valuefactory_seen_,
          "tao/Valuetype/ValueFactory.h",
          this->client_header_
        );
    }

  // This is true if we have a typecode or TCKind in the IDL file.
  // If not included here, it will appear in *C.cpp, if TCs not suppressed.
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.typecode_seen_,
      "tao/Typecode.h",
      this->client_header_
    );

  // This is true if we have an 'any' in the IDL file.
  // If not included here, it will appear in *C.cpp, if Anys not suppressed.
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.any_seen_,
      "tao/Any.h",
      this->client_header_
    );

  // ParameterMode is so rarely used, it was put in a separate TAO file.
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.parametermode_seen_,
      "tao/ParameterMode.h",
      this->client_header_
    );

  // Include the Messaging library entry point, if AMI is enabled.
  if (be_global->ami_call_back () == I_TRUE)
    {
      // Include Messaging skeleton file.
      this->gen_standard_include (this->client_header_,
                                  "tao/Messaging/Messaging.h");
    }

  // Include the smart proxy base class if smart proxies are enabled.
  if (be_global->gen_smart_proxies () == I_TRUE)
    {
      this->gen_standard_include (this->client_header_,
                                  "tao/SmartProxies/Smart_Proxies.h");
    }

  // Must have knowledge of the base class.
  this->gen_seq_file_includes ();

  // _vars and _outs are typedefs of template class instantiations.
  this->gen_var_file_includes ();
}

void
TAO_CodeGen::gen_stub_src_includes (void)
{
  // Generate the include statement for the precompiled header file.
  if (be_global->pch_include ())
    {
      *this->client_stubs_ << "#include \""
                           << be_global->pch_include ()
                           << "\"";
    }

  // Generate the include statement for the client header. We just
  // need to put only the base names. Path info is not required.
  *this->client_stubs_ << "\n#include \""
                       << be_global->be_get_client_hdr_fname (1)
                       << "\"";

  // Conditional includes.

  // Operations for local interfaces are pure virtual.
  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.non_local_op_seen_))
    {
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Exception_Data.h");
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Invocation_Adapter.h");
    }

  // Any abstract interface present will probably have an operation.
  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                        idl_global->decls_seen_masks.abstract_iface_seen_))
    {
      this->gen_standard_include (
          this->client_stubs_,
          "tao/Valuetype/AbstractBase_Invocation_Adapter.h"
        );

      this->gen_standard_include (
          this->client_stubs_,
          "tao/Valuetype/AbstractBase_T.h"
        );
    }

  if (be_global->ami_call_back () == I_TRUE)
    {
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Messaging/Asynch_Invocation_Adapter.h");

      // If a valuetype has been seen, this will already be in the header file.
      if (!ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                            idl_global->decls_seen_masks.valuetype_seen_))
        {
          // For AMI exception holders.
          this->gen_standard_include (this->client_stubs_,
                                      "tao/Valuetype/ValueBase.h");
          this->gen_standard_include (this->client_stubs_,
                                      "tao/Valuetype/Valuetype_Adapter_Impl.h");
        }
    }

  // If valuefactory_seen_ was set, this was generated in the stub header file,
  // otherwise it needs to go here - used in _tao_unmarshal().
  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.valuetype_seen_)
      && !ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                           idl_global->decls_seen_masks.valuefactory_seen_))
    {
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Valuetype/ValueFactory.h");
    }

  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.non_local_iface_seen_))
    {
      // Needed for _narrow(), which is now template-based.
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Object_T.h");
    }

  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.octet_seq_seen_))
    {
      // Needed for the TAO_NO_COPY_OCTET_SEQUENCES optimization. Note that
      // it is preferable to just refer to CORBA::OctetSeq in the IDL file.
      this->gen_standard_include (this->client_stubs_,
                                  "tao/ORB_Core.h");
    }

  // We generate this include if we have typecode support and have not
  // already included it in the header file.
  if (!ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                        idl_global->decls_seen_masks.typecode_seen_)
      && be_global->tc_support ())
    {
      this->gen_standard_include (this->client_stubs_,
                                  "tao/Typecode.h");
    }

  // Includes whatever Any template classes that may be needed.
  this->gen_any_file_includes ();

  // Includes whatever arg helper template classes that may be needed.
  this->gen_arg_file_includes (this->client_stubs_);

  // strcmp() is used with interfaces and exceptions.
  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                       idl_global->decls_seen_masks.interface_seen_)
      || ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                          idl_global->decls_seen_masks.exception_seen_)
      || ACE_BIT_ENABLED (idl_global->decls_seen_info_,
                          idl_global->decls_seen_masks.union_seen_))
    {
      // Needed for _narrow(), which is now template-based.
      this->gen_standard_include (this->client_stubs_,
                                  "ace/OS_NS_string.h");
    }

  if (be_global->gen_amh_classes () == I_TRUE)
    {
      *this->client_stubs_ << be_nl;

      // Necessary for the AIX compiler.
      this->gen_standard_include (this->client_stubs_,
                                  "ace/Auto_Ptr.h");
    }
}

void
TAO_CodeGen::gen_skel_src_includes (void)
{
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PortableServer/Object_Adapter.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PortableServer/Operation_Table.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/TAO_Server_Request.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/ORB_Core.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/Profile.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/Stub.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/IFR_Client_Adapter.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/Object_T.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/Typecode.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/DynamicC.h");

  if (be_global->gen_thru_poa_collocation ()
      || be_global->gen_direct_collocation ())
    {
      this->gen_arg_file_includes (this->server_skeletons_);
    }

  // The following header must always be included.
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PortableInterceptor.h");

  // Include Portable Interceptor related headers.
  *this->server_skeletons_ << "\n#if TAO_HAS_INTERCEPTORS == 1";
  this->gen_standard_include (this->server_skeletons_,
                              "tao/RequestInfo_Util.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PICurrent.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PortableServer/ServerRequestInfo.h");
  this->gen_standard_include (this->server_skeletons_,
                              "tao/PortableServer/ServerInterceptorAdapter.h");
  *this->server_skeletons_ << "\n#endif  /* TAO_HAS_INTERCEPTORS == 1 */\n";

  this->gen_standard_include (this->server_skeletons_,
                              "ace/Dynamic_Service.h");
  
  // For Static_Allocator_Base
  this->gen_standard_include (this->server_skeletons_,
                              "ace/Malloc_Allocator.h");
  // To get ACE_UNUSED_ARGS
  this->gen_standard_include (this->server_skeletons_,
                              "ace/config-all.h");
}

void
TAO_CodeGen::gen_seq_file_includes (void)
{
  // @@@ (JP) These can get more specialized, after the TAO seq template
  // files have been split up.

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.vt_seq_seen_,
      "tao/Valuetype/Sequence_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.seq_seen_,
      "tao/Sequence_T.h",
      this->client_header_
    );
}

void
TAO_CodeGen::gen_any_file_includes (void)
{
  if (be_global->any_support ())
    {
      TAO_OutStream *stream = this->client_stubs_;

      if (be_global->gen_anyop_files ())
        {
          stream = this->anyop_source_;
        }

      this->gen_cond_file_include (
          idl_global->decls_seen_masks.interface_seen_
          | idl_global->decls_seen_masks.valuetype_seen_,
          "tao/Any_Impl_T.h",
          stream
        );

      this->gen_cond_file_include (
          idl_global->decls_seen_masks.aggregate_seen_
          | idl_global->decls_seen_masks.seq_seen_
          | idl_global->decls_seen_masks.exception_seen_,
          "tao/Any_Dual_Impl_T.h",
          stream
        );

      this->gen_cond_file_include (
          idl_global->decls_seen_masks.array_seen_,
          "tao/Any_Array_Impl_T.h",
          stream
        );

      this->gen_cond_file_include (
          idl_global->decls_seen_masks.enum_seen_,
          "tao/Any_Basic_Impl_T.h",
          stream
        );
    }
}

void
TAO_CodeGen::gen_var_file_includes (void)
{
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.interface_seen_
      | idl_global->decls_seen_masks.fwd_iface_seen_,
      "tao/Objref_VarOut_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.valuetype_seen_
      | idl_global->decls_seen_masks.fwd_valuetype_seen_,
      "tao/Valuetype/Value_VarOut_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.seq_seen_,
      "tao/Seq_Var_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.seq_seen_,
      "tao/Seq_Out_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.aggregate_seen_,
      "tao/VarOut_T.h",
      this->client_header_
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.array_seen_,
      "tao/Array_VarOut_T.h",
      this->client_header_
    );
}

void
TAO_CodeGen::gen_arg_file_includes (TAO_OutStream *stream)
{
  this->gen_cond_file_include (
      idl_global->decls_seen_masks.basic_arg_seen_,
      "tao/Basic_Arguments.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.bd_string_arg_seen_,
      "tao/BD_String_Argument_T.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.fixed_array_arg_seen_,
      "tao/Fixed_Array_Argument_T.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.fixed_size_arg_seen_,
      "tao/Fixed_Size_Argument_T.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.object_arg_seen_,
      "tao/Object_Argument_T.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.special_basic_arg_seen_,
      "tao/Special_Basic_Arguments.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.ub_string_arg_seen_,
      "tao/UB_String_Arguments.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.var_array_arg_seen_,
      "tao/Var_Array_Argument_T.h",
      stream
    );

  this->gen_cond_file_include (
      idl_global->decls_seen_masks.var_size_arg_seen_,
      "tao/Var_Size_Argument_T.h",
      stream
    );
}

void
TAO_CodeGen::gen_cond_file_include (ACE_UINT64 mask,
                                    const char *filepath,
                                    TAO_OutStream *stream)
{
  if (ACE_BIT_ENABLED (idl_global->decls_seen_info_, mask))
    {
      this->gen_standard_include (stream,
                                  filepath);
    }
}
