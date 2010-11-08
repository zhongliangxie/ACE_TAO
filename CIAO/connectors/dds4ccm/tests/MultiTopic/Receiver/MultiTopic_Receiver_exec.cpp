// -*- C++ -*-
// $Id$

/**
 * Code generated by the The ACE ORB (TAO) IDL Compiler v1.8.2
 * TAO and the TAO IDL Compiler have been developed by:
 *       Center for Distributed Object Computing
 *       Washington University
 *       St. Louis, MO
 *       USA
 *       http://www.cs.wustl.edu/~schmidt/doc-center.html
 * and
 *       Distributed Object Computing Laboratory
 *       University of California at Irvine
 *       Irvine, CA
 *       USA
 * and
 *       Institute for Software Integrated Systems
 *       Vanderbilt University
 *       Nashville, TN
 *       USA
 *       http://www.isis.vanderbilt.edu/
 *
 * Information about TAO is available at:
 *     http://www.cs.wustl.edu/~schmidt/TAO.html
 **/

#include "MultiTopic_Receiver_exec.h"
#include "tao/ORB_Core.h"
#include "ace/OS_NS_time.h"
#include "ace/Reactor.h"

namespace CIAO_Shapes_Receiver_Impl
{

  //============================================================
  // read_action_Generator
  //============================================================
  read_action_Generator::read_action_Generator (Receiver_exec_i &callback)
    : pulse_callback_ (callback)
  {
  }

  read_action_Generator::~read_action_Generator ()
  {
  }

  int
  read_action_Generator::handle_timeout (const ACE_Time_Value &, const void *)
  {
    if (pulse_callback_.read_data ())
      {
        this->pulse_callback_.read_one();
//      this->pulse_callback_.read_all();
      }
    if (pulse_callback_.get_data ())
      {
        this->pulse_callback_.get_one ();
//      this->pulse_callback_.get_all ();
      }
    return 0;
  }

  /**
   * Facet Executor Implementation Class: info_out_data_listener_exec_i
  */
  info_out_data_listener_exec_i::info_out_data_listener_exec_i (
        ::Shapes::CCM_Receiver_Context_ptr ctx)
    : ciao_context_ (
        ::Shapes::CCM_Receiver_Context::_duplicate (ctx))
  {
  }

  info_out_data_listener_exec_i::~info_out_data_listener_exec_i (void)
  {
  }

  // Operations from ::Shapes::Listener

  void
  info_out_data_listener_exec_i::on_one_data (const ::ShapeType & datum,
  const ::CCM_DDS::ReadInfo & /* info */)
  {
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("ShapeType_Listener: ")
            ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
            datum.color.in (),
            datum.x,
            datum.y,
            datum.shapesize));
  }

  void
  info_out_data_listener_exec_i::on_many_data (const ::ShapeTypeSeq & /* data */,
  const ::CCM_DDS::ReadInfoSeq & /* infos */)
  {
    /* Your code here. */
  }

  /**
   * Facet Executor Implementation Class: info_out_status_exec_i
   */
  info_out_status_exec_i::info_out_status_exec_i (
        ::Shapes::CCM_Receiver_Context_ptr ctx)
    : ciao_context_ (
        ::Shapes::CCM_Receiver_Context::_duplicate (ctx))
  {
  }

  info_out_status_exec_i::~info_out_status_exec_i (void)
  {
  }

  // Operations from ::CCM_DDS::PortStatusListener
  void
  info_out_status_exec_i::on_requested_deadline_missed (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::RequestedDeadlineMissedStatus & /* status */)
  {
    /* Your code here. */
  }

  void
  info_out_status_exec_i::on_sample_lost (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::SampleLostStatus & /* status */)
  {
    /* Your code here. */
  }

  /**
   * Facet Executor Implementation Class: info_get_status_exec_i
   */
  info_get_status_exec_i::info_get_status_exec_i (
        ::Shapes::CCM_Receiver_Context_ptr ctx)
    : ciao_context_ (
        ::Shapes::CCM_Receiver_Context::_duplicate (ctx))
  {
  }

  info_get_status_exec_i::~info_get_status_exec_i (void)
  {
  }

  // Operations from ::CCM_DDS::PortStatusListener

  void
  info_get_status_exec_i::on_requested_deadline_missed (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::RequestedDeadlineMissedStatus & /* status */)
  {
    /* Your code here. */
  }

  void
  info_get_status_exec_i::on_sample_lost (::DDS::DataReader_ptr /* the_reader */,
  const ::DDS::SampleLostStatus & /* status */)
  {
    /* Your code here. */
  }

  /**
   * Component Executor Implementation Class: Receiver_exec_i
   */

  Receiver_exec_i::Receiver_exec_i (void)
    : rate_ (0),
      get_data_ (false),
      read_data_ (false),
      raw_listen_ (false)
  {
  }

  Receiver_exec_i::~Receiver_exec_i (void)
  {
  }

  // Supported operations and attributes.
  void
  Receiver_exec_i::read_one (void)
  {
    ShapeType  shape_info;
    shape_info.color = "GREEN";
    ::CCM_DDS::ReadInfo readinfo;

    try
      {
        this->reader_sq_->read_one_last (shape_info, readinfo, ::DDS::HANDLE_NIL);
        time_t tim = readinfo.source_timestamp.sec;
        tm* time = ACE_OS::localtime(&tim);
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ_ONE square_Info ")
                  ACE_TEXT (" -> date = %02d:%02d:%02d.%d\n"),
                            time->tm_hour,
                            time->tm_min,
                            time->tm_sec,
                            readinfo.source_timestamp.nanosec));
        ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ ON square info : ")
            ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
            shape_info.color.in (),
            shape_info.x,
            shape_info.y,
            shape_info.shapesize));

        this->reader_tr_->read_one_last (shape_info, readinfo,
                                         ::DDS::HANDLE_NIL);
         tim = readinfo.source_timestamp.sec;
         time = ACE_OS::localtime(&tim);
         ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ_ONE triangle_Info ")
                   ACE_TEXT (" -> date = %02d:%02d:%02d.%d\n"),
                             time->tm_hour,
                             time->tm_min,
                             time->tm_sec,
                             readinfo.source_timestamp.nanosec));
         ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ ON triangle info : ")
             ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
             shape_info.color.in (),
             shape_info.x,
             shape_info.y,
             shape_info.shapesize));

         this->reader_cq_->read_one_last (shape_info, readinfo,
                                          ::DDS::HANDLE_NIL);
         tim = readinfo.source_timestamp.sec;
         time = ACE_OS::localtime(&tim);
          ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ_ONE circle_Info ")
                    ACE_TEXT (" -> date = %02d:%02d:%02d.%d\n"),
                              time->tm_hour,
                              time->tm_min,
                              time->tm_sec,
                              readinfo.source_timestamp.nanosec));
          ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("READ ON circle info : ")
              ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
              shape_info.color.in (),
              shape_info.x,
              shape_info.y,
              shape_info.shapesize));
      }
    catch(const CCM_DDS::NonExistent& )
      {
        ACE_ERROR ((LM_ERROR, ACE_TEXT ("ShapeType_Read_One: ")
                  ACE_TEXT ("no shape_info receieved\n")));
      }
  }

  void
   Receiver_exec_i::get_one (void)
   {
     ShapeType_var shape_info;
     ::CCM_DDS::ReadInfo_var readinfo;

     try
       {
         if (this->getter_sq_->get_one (shape_info.out (), readinfo.out ()))
           {
             time_t tim = readinfo->source_timestamp.sec;
             tm* time = ACE_OS::localtime(&tim);
             ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE ReadInfo square-> ")
                                    ACE_TEXT ("date = %02d:%02d:%02d.%d\n"),
                                 time->tm_hour,
                                 time->tm_min,
                                 time->tm_sec,
                                 readinfo->source_timestamp.nanosec));
             ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE ShapeType square : ")
                                    ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
                 shape_info->color.in (),
                 shape_info->x,
                 shape_info->y,
                 shape_info->shapesize));
           }
         else
           {
             ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE square No data available for <%C>\n"),
                     shape_info->color.in ()));
           }
         if (this->getter_tr_->get_one (shape_info.out (), readinfo.out ()))
             {
               time_t tim = readinfo->source_timestamp.sec;
               tm* time = ACE_OS::localtime(&tim);
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE ReadInfo triangle-> ")
                                      ACE_TEXT ("date = %02d:%02d:%02d.%d\n"),
                                   time->tm_hour,
                                   time->tm_min,
                                   time->tm_sec,
                                   readinfo->source_timestamp.nanosec));
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE ShapeType triangle: ")
                                      ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
                   shape_info->color.in (),
                   shape_info->x,
                   shape_info->y,
                   shape_info->shapesize));
             }
           else
             {
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE triangle No data available for <%C>\n"),
                       shape_info->color.in ()));
             }

         if (this->getter_cq_->get_one (shape_info.out (), readinfo.out ()))
             {
               time_t tim = readinfo->source_timestamp.sec;
               tm* time = ACE_OS::localtime(&tim);
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE circle ReadInfo -> ")
                                      ACE_TEXT ("date = %02d:%02d:%02d.%d\n"),
                                   time->tm_hour,
                                   time->tm_min,
                                   time->tm_sec,
                                   readinfo->source_timestamp.nanosec));
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE ShapeType  circle: ")
                                      ACE_TEXT ("received shape_info for <%C> at %u:%u:%u\n"),
                   shape_info->color.in (),
                   shape_info->x,
                   shape_info->y,
                   shape_info->shapesize));
             }
           else
             {
               ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("GET_ONE  circle No data available for <%C>\n"),
                       shape_info->color.in ()));
             }
       }
     catch(const CCM_DDS::NonExistent& )
       {
         ACE_ERROR ((LM_ERROR, ACE_TEXT ("ShapeType_Read_One: no shape_info receieved\n")));
       }
   }
  // Supported operations and attributes.
  ACE_Reactor*
  Receiver_exec_i::reactor (void)
  {
    ACE_Reactor* reactor = 0;
    ::CORBA::Object_var ccm_object =
      this->ciao_context_->get_CCM_object();
    if (! ::CORBA::is_nil (ccm_object.in ()))
      {
        ::CORBA::ORB_var orb = ccm_object->_get_orb ();
        if (! ::CORBA::is_nil (orb.in ()))
          {
            reactor = orb->orb_core ()->reactor ();
          }
      }
    if (reactor == 0)
      {
        throw ::CORBA::INTERNAL ();
      }
    return reactor;
  }
  void
  Receiver_exec_i::start (void)
  {
    // calculate the interval time
    long const usec = 1000000 / this->rate_;

    if (this->reactor ()->schedule_timer (
                this->ticker_,
                0,
                ACE_Time_Value (0, usec),
                ACE_Time_Value (0, usec)) == -1)
    {
      ACE_ERROR ((LM_ERROR, ACE_TEXT ("Sender_exec_i::start : ")
                             ACE_TEXT ("Error scheduling timer")));
    }
  }

    void
    Receiver_exec_i::stop (void)
    {
      this->reactor ()->cancel_timer (this->ticker_);
      delete this->ticker_;
     }
  // Component attributes and port operations.

  ::Shapes::DDS_Typed::CCM_Listener_ptr
  Receiver_exec_i::get_info_out_sq_data_listener (void)
  {
    if ( ::CORBA::is_nil (this->ciao_info_out_data_listener_.in ()))
      {
        info_out_data_listener_exec_i *tmp = 0;
        ACE_NEW_RETURN (
          tmp,
          info_out_data_listener_exec_i (
            this->ciao_context_.in ()),
            ::Shapes::DDS_Typed::CCM_Listener::_nil ());

          this->ciao_info_out_data_listener_ = tmp;
      }

    return
      ::Shapes::DDS_Typed::CCM_Listener::_duplicate (
        this->ciao_info_out_data_listener_.in ());
  }

  ::Shapes::DDS_Typed::CCM_Listener_ptr
   Receiver_exec_i::get_info_out_tr_data_listener (void)
   {
     if ( ::CORBA::is_nil (this->ciao_info_out_data_listener_.in ()))
       {
         info_out_data_listener_exec_i *tmp = 0;
         ACE_NEW_RETURN (
           tmp,
           info_out_data_listener_exec_i (
             this->ciao_context_.in ()),
             ::Shapes::DDS_Typed::CCM_Listener::_nil ());

           this->ciao_info_out_data_listener_ = tmp;
       }

     return
       ::Shapes::DDS_Typed::CCM_Listener::_duplicate (
         this->ciao_info_out_data_listener_.in ());
   }

  ::Shapes::DDS_Typed::CCM_Listener_ptr
   Receiver_exec_i::get_info_out_cq_data_listener (void)
   {
     if ( ::CORBA::is_nil (this->ciao_info_out_data_listener_.in ()))
       {
         info_out_data_listener_exec_i *tmp = 0;
         ACE_NEW_RETURN (
           tmp,
           info_out_data_listener_exec_i (
             this->ciao_context_.in ()),
             ::Shapes::DDS_Typed::CCM_Listener::_nil ());

           this->ciao_info_out_data_listener_ = tmp;
       }

     return
       ::Shapes::DDS_Typed::CCM_Listener::_duplicate (
         this->ciao_info_out_data_listener_.in ());
   }

  ::CCM_DDS::CCM_PortStatusListener_ptr
  Receiver_exec_i::get_info_out_sq_status (void)
  {
    if ( ::CORBA::is_nil (this->ciao_info_out_status_.in ()))
      {
        info_out_status_exec_i *tmp = 0;
        ACE_NEW_RETURN (
          tmp,
          info_out_status_exec_i (
            this->ciao_context_.in ()),
            ::CCM_DDS::CCM_PortStatusListener::_nil ());

          this->ciao_info_out_status_ = tmp;
      }

    return
      ::CCM_DDS::CCM_PortStatusListener::_duplicate (
        this->ciao_info_out_status_.in ());
  }

  ::CCM_DDS::CCM_PortStatusListener_ptr
    Receiver_exec_i::get_info_out_tr_status (void)
    {
      if ( ::CORBA::is_nil (this->ciao_info_out_status_.in ()))
        {
          info_out_status_exec_i *tmp = 0;
          ACE_NEW_RETURN (
            tmp,
            info_out_status_exec_i (
              this->ciao_context_.in ()),
              ::CCM_DDS::CCM_PortStatusListener::_nil ());

            this->ciao_info_out_status_ = tmp;
        }

      return
        ::CCM_DDS::CCM_PortStatusListener::_duplicate (
          this->ciao_info_out_status_.in ());
    }

  ::CCM_DDS::CCM_PortStatusListener_ptr
    Receiver_exec_i::get_info_out_cq_status (void)
    {
      if ( ::CORBA::is_nil (this->ciao_info_out_status_.in ()))
        {
          info_out_status_exec_i *tmp = 0;
          ACE_NEW_RETURN (
            tmp,
            info_out_status_exec_i (
              this->ciao_context_.in ()),
              ::CCM_DDS::CCM_PortStatusListener::_nil ());

            this->ciao_info_out_status_ = tmp;
        }

      return
        ::CCM_DDS::CCM_PortStatusListener::_duplicate (
          this->ciao_info_out_status_.in ());
    }


  ::CCM_DDS::CCM_PortStatusListener_ptr
  Receiver_exec_i::get_info_get_sq_status (void)
  {
    if ( ::CORBA::is_nil (this->ciao_info_get_status_.in ()))
      {
        info_get_status_exec_i *tmp = 0;
        ACE_NEW_RETURN (
          tmp,
          info_get_status_exec_i (
            this->ciao_context_.in ()),
            ::CCM_DDS::CCM_PortStatusListener::_nil ());

          this->ciao_info_get_status_ = tmp;
      }

    return
      ::CCM_DDS::CCM_PortStatusListener::_duplicate (
        this->ciao_info_get_status_.in ());
  }

  ::CCM_DDS::CCM_PortStatusListener_ptr
    Receiver_exec_i::get_info_get_tr_status (void)
    {
      if ( ::CORBA::is_nil (this->ciao_info_get_status_.in ()))
        {
          info_get_status_exec_i *tmp = 0;
          ACE_NEW_RETURN (
            tmp,
            info_get_status_exec_i (
              this->ciao_context_.in ()),
              ::CCM_DDS::CCM_PortStatusListener::_nil ());

            this->ciao_info_get_status_ = tmp;
        }

      return
        ::CCM_DDS::CCM_PortStatusListener::_duplicate (
          this->ciao_info_get_status_.in ());
    }

  ::CCM_DDS::CCM_PortStatusListener_ptr
    Receiver_exec_i::get_info_get_cq_status (void)
    {
      if ( ::CORBA::is_nil (this->ciao_info_get_status_.in ()))
        {
          info_get_status_exec_i *tmp = 0;
          ACE_NEW_RETURN (
            tmp,
            info_get_status_exec_i (
              this->ciao_context_.in ()),
              ::CCM_DDS::CCM_PortStatusListener::_nil ());

            this->ciao_info_get_status_ = tmp;
        }

      return
        ::CCM_DDS::CCM_PortStatusListener::_duplicate (
          this->ciao_info_get_status_.in ());
    }

  ::CORBA::ULong
  Receiver_exec_i::rate (void)
  {
    return this->rate_;
  }

  void
  Receiver_exec_i::rate (
    const ::CORBA::ULong rate)
  {
    this->rate_ = rate;
  }

  ::CORBA::Boolean
  Receiver_exec_i::get_data (void)
  {
    return this->get_data_;
  }

  void
  Receiver_exec_i::get_data (
    const ::CORBA::Boolean get_data)
  {
    this->get_data_ = get_data;
  }

  ::CORBA::Boolean
  Receiver_exec_i::read_data (void)
  {
    return this->read_data_;
  }

  void
  Receiver_exec_i::read_data (
    const ::CORBA::Boolean read_data)
  {
    this->read_data_ = read_data;
  }

  ::CORBA::Boolean
  Receiver_exec_i::raw_listen (void)
  {
    return this->raw_listen_;
  }

  void
  Receiver_exec_i::raw_listen (
    const ::CORBA::Boolean raw_listen)
  {
    this->raw_listen_ = raw_listen;
  }

  // Operations from Components::SessionComponent.

  void
  Receiver_exec_i::set_session_context (
    ::Components::SessionContext_ptr ctx)
  {
    ACE_DEBUG ((LM_DEBUG,
           ACE_TEXT (" 222 Receiver_exec_i::set_session_context\n")));

    this->ciao_context_ =
      ::Shapes::CCM_Receiver_Context::_narrow (ctx);

    if ( ::CORBA::is_nil (this->ciao_context_.in ()))
      {
        throw ::CORBA::INTERNAL ();
      }
  }

  void
  Receiver_exec_i::configuration_complete (void)
  {
    ACE_DEBUG ((LM_DEBUG,
           ACE_TEXT (" 333 Receiver_exec_i::configuration_complete\n")));

    if (this->read_data ())
       {
         this->reader_sq_ = this->ciao_context_->get_connection_info_out_sq_data ();
         if (::CORBA::is_nil (this->reader_sq_.in ()))
            {
              ACE_ERROR ((LM_INFO,
                      ACE_TEXT ("Error:  get_connection_info_out_sq_data is null!\n")));
              throw ::CORBA::INTERNAL ();
            }
         this->reader_tr_ = this->ciao_context_->get_connection_info_out_tr_data ();
         this->reader_cq_ = this->ciao_context_->get_connection_info_out_cq_data ();

       }
     if (this->get_data ())
       {
         this->getter_sq_ = this->ciao_context_->get_connection_info_get_sq_fresh_data ();
         this->getter_tr_ = this->ciao_context_->get_connection_info_get_tr_fresh_data ();
         this->getter_cq_ = this->ciao_context_->get_connection_info_get_cq_fresh_data ();
       }
  }

  void
  Receiver_exec_i::ccm_activate (void)
  {
    ACE_DEBUG ((LM_DEBUG,
           ACE_TEXT ("    444Receiver_exec_i::ccm_activate\n")));
    ::CCM_DDS::DataListenerControl_var lc_sq =
    this->ciao_context_->get_connection_info_out_sq_data_control ();
    if (::CORBA::is_nil (lc_sq.in ()))
      {
        ACE_ERROR ((LM_INFO,
                ACE_TEXT ("Error:  Listener control receptacle sq is null!\n")));
        throw ::CORBA::INTERNAL ();
      }

    lc_sq->mode (::CCM_DDS::ONE_BY_ONE);

    ::CCM_DDS::DataListenerControl_var lc_tr =
    this->ciao_context_->get_connection_info_out_tr_data_control ();
    if (::CORBA::is_nil (lc_tr.in ()))
      {
        ACE_ERROR ((LM_INFO,
            ACE_TEXT ("Error:  Listener control receptacle tr is null!\n")));
          throw ::CORBA::INTERNAL ();
      }
    lc_tr->mode (::CCM_DDS::ONE_BY_ONE);

    ::CCM_DDS::DataListenerControl_var lc_cq =
    this->ciao_context_->get_connection_info_out_cq_data_control ();
    if (::CORBA::is_nil (lc_cq.in ()))
      {
        ACE_ERROR ((LM_INFO,
            ACE_TEXT ("Error:  Listener control receptacle cq is null!\n")));
           throw ::CORBA::INTERNAL ();
      }

    lc_cq->mode (::CCM_DDS::ONE_BY_ONE);
//  this->start();
    ACE_DEBUG ((LM_DEBUG,
           ACE_TEXT ("   555Receiver_exec_i::ccm_activate\n")));
  }

  void
  Receiver_exec_i::ccm_passivate (void)
  {
//  this->stop();
  }

  void
  Receiver_exec_i::ccm_remove (void)
  {
    /* Your code here. */
  }

  extern "C" RECEIVER_EXEC_Export  ::Components::EnterpriseComponent_ptr
  create_Shape_Receiver_Impl (void)
  {
    ACE_DEBUG ((LM_DEBUG, ACE_TEXT ("111   create_Shape_Receiver_Impl\n")));

    ::Components::EnterpriseComponent_ptr retval =
      ::Components::EnterpriseComponent::_nil ();

    ACE_NEW_NORETURN (
      retval,
      Receiver_exec_i);

    return retval;
  }
}
