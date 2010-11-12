// -*- C++ -*-
// $Id$

#include "dds4ccm/impl/DataReaderStateListener_T.h"
#include "dds4ccm/impl/StateListenerControl_T.h"
#include "dds4ccm/impl/logger/Log_Macros.h"

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::DDS_StateListen_T (void)
{
}

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::~DDS_StateListen_T (void)
{
}

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
void
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::set_component (
    ::CORBA::Object_ptr component)
{
  DDS_Subscriber_Base_T<DDS_TYPE, CCM_TYPE, FIXED>::set_component (component);
  this->data_control_->_set_component (component);
}

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
void
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::activate (
  typename CCM_TYPE::statelistener_type::_ptr_type listener,
  ::CCM_DDS::PortStatusListener_ptr status,
  ACE_Reactor* reactor)
{
  DDS4CCM_TRACE ("DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::activate");

  ::DDS::StatusMask const mask =
    DataReaderStateListener_type::get_mask (listener);

  if (mask != 0)
    {
      if (::CORBA::is_nil (this->listener_.in ()))
        {
          ACE_NEW_THROW_EX (this->listener_,
                            DataReaderStateListener_type (
                              listener,
                              status,
                              this->data_control_,
                              reactor,
                              this->condition_manager_),
                            ::CORBA::NO_MEMORY ());
        }

      ::DDS::DataReader_var dr = this->dds_read_->get_dds_reader ();
      if (!::CORBA::is_nil (dr.in ()))
        {
          ::DDS::ReturnCode_t const retcode =
            dr->set_listener (this->listener_.in (), mask);

          if (retcode != ::DDS::RETCODE_OK)
            {
              DDS4CCM_ERROR (DDS4CCM_LOG_LEVEL_ERROR, (LM_ERROR, DDS4CCM_INFO
                            "DDS_StateListen_T::activate - "
                            "Error setting the listener on the DataReader - <%C>\n",
                            ::CIAO::DDS4CCM::translate_retcode (retcode)));
              throw ::CORBA::INTERNAL ();
            }
        }
      else
        {
          DDS4CCM_ERROR (DDS4CCM_LOG_LEVEL_ERROR, (LM_ERROR, DDS4CCM_INFO
                        "DDS_StateListen_T::activate - "
                        "Error while retrieving the DataReader\n"));
          throw ::CORBA::INTERNAL ();
        }
    }
}

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
void
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::remove (
  ::DDS::Subscriber_ptr subscriber)
{
  DDS4CCM_TRACE ("DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::remove");

  this->data_control_->_set_component (::CORBA::Object::_nil ());
  DDSSubscriberBase_type::remove (subscriber);
}

template <typename DDS_TYPE, typename CCM_TYPE, bool FIXED>
::CCM_DDS::CCM_StateListenerControl_ptr
DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::get_data_control (void)
{
  DDS4CCM_TRACE ("DDS_StateListen_T<DDS_TYPE, CCM_TYPE, FIXED>::get_data_control");

  return ::CCM_DDS::CCM_StateListenerControl::_duplicate (this->data_control_);
}


