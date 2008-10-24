// $Id$

#include "XML_Error_Handler.h"
#include "ace/Log_Msg.h"
#include "ace/Auto_Ptr.h"
#include <xercesc/util/XMLString.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/sax/SAXParseException.hpp>
#include "XercesString.h"

using xercesc::SAXParseException;

namespace CIAO
{
  namespace XML
  {
    XML_Error_Handler::XML_Error_Handler (void)
      : errors_ (false)
    {
    }

    XML_Error_Handler::~XML_Error_Handler()
    {
    }

    void XML_Error_Handler::warning(const SAXParseException& toCatch)
    {
      XStr file (toCatch.getSystemId ());
      XStr msg (toCatch.getMessage ());
      
      std::cerr << "Warning: " << file << ':' << toCatch.getLineNumber ()
                << ':' << toCatch.getColumnNumber () << " - "
                << msg << std::endl;
    }
    
    void XML_Error_Handler::error(const SAXParseException& toCatch)
    {
      XStr file (toCatch.getSystemId ());
      XStr msg (toCatch.getMessage ());
      
      std::cerr << "Error: " << file << ':' << toCatch.getLineNumber ()
                << ':' << toCatch.getColumnNumber () << " - "
                << msg << std::endl;
      this->errors_ = true;
    }
    
    void XML_Error_Handler::fatalError(const SAXParseException& toCatch)
    {
      XStr file (toCatch.getSystemId ());
      XStr msg (toCatch.getMessage ());
      
      std::cerr << "Fatal Error: " << file << ':' << toCatch.getLineNumber ()
                << ':' << toCatch.getColumnNumber () << " - "
                << msg << std::endl;
      this->errors_ = true;      
    }
    
    void XML_Error_Handler::resetErrors()
    {
      this->errors_ = false;
    }

    bool
    XML_Error_Handler::getErrors (void) const
    {
      return this->errors_;
    }
  }
}
