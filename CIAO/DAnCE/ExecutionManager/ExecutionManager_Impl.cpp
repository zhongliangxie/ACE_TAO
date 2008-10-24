//--*C++*--
// $Id$

#include "ExecutionManager_Impl.h"
#include "ace/Log_Msg.h"
#include "tools/Config_Handlers/DnC_Dump.h"
#include "DAnCE/Logger/Log_Macros.h"


using namespace DAnCE;

ExecutionManager_Impl::ExecutionManager_Impl (CORBA::ORB_ptr orb,
                                              PortableServer::POA_ptr poa)
    : orb_ (CORBA::ORB::_duplicate (orb))
    , poa_ (PortableServer::POA::_duplicate (poa))
{
  DANCE_TRACE (DLINFO "ExecutionManager_Impl::ExecutionManager_Impl");
}

ExecutionManager_Impl::~ExecutionManager_Impl()
{
  DANCE_TRACE (DLINFO "ExecutionManager_Impl::~ExecutionManager_Impl");
  for (TDomainManagers::iterator iter = this->managers_.begin();
       iter != this->managers_.end();
       ++iter)
    {
      DANCE_DEBUG ((LM_TRACE, DLINFO "ExecutionManager_Impl::~ExecutionManager_Impl - "
                    "deactivating DAM \"%s\"\n", (*iter).ext_id_.c_str()));
      PortableServer::ObjectId_var id = this->poa_->servant_to_id ( (*iter).int_id_);
      DANCE_DEBUG ((LM_TRACE, DLINFO "ExecutionManager_Impl::~ExecutionManager_Impl - "
                    "before deactivate_object...\n"));
      this->poa_->deactivate_object (id.in());
      DANCE_DEBUG ((LM_TRACE, DLINFO "ExecutionManager_Impl::~ExecutionManager_Impl - "
                    "deleting DomainApplicationManager\n"));
      delete (*iter).int_id_;
      DANCE_DEBUG ((LM_TRACE, DLINFO "ExecutionManager_Impl::~ExecutionManager_Impl - "
                    "DomainApplicationManager deleted\n"));
    }
}

::Deployment::DomainApplicationManager_ptr
ExecutionManager_Impl::preparePlan (const ::Deployment::DeploymentPlan & plan,
                                    ::Deployment::ResourceCommitmentManager_ptr /*resourceCommitment*/)
{
  DANCE_TRACE (DLINFO "ExecutionManager_Impl::preparePlan");

  // Check if plan is already deployed. 
  DomainApplicationManager_Impl * dam = 0;
  if (0 == this->managers_.find (plan.UUID.in(), dam))
    {
      DANCE_DEBUG((LM_NOTICE, DLINFO "ExecutionManager_Impl::preparePlan - "
                   "DomainApplicationManager with specified UUID already exists\n"));
      // Should we return on this situation reference on existed DomainApplicationManager or
      // we should throw PlanError exception?
      CORBA::Object_var ref = this->poa_->servant_to_reference (dam);
      return Deployment::DomainApplicationManager::_narrow (ref.in ());
    }


  DomainApplicationManager_Impl* dam_servant;
  ACE_NEW_THROW_EX (dam_servant,
                    DAnCE::DomainApplicationManager_Impl (this->orb_.in (),
                                                          this->poa_.in (),
                                                          plan,
                                                          this->nodes_),
                    CORBA::NO_MEMORY ());
  this->managers_.rebind (plan.UUID.in(), dam_servant);
  DANCE_DEBUG((LM_TRACE, DLINFO "ExecutionManager_Impl::preparePlan - "
               "Domain Application Manager  was successfully created.\n"));

  PortableServer::ObjectId_var id = this->poa_->activate_object (dam_servant);

  DANCE_DEBUG((LM_NOTICE, DLINFO "ExecutionManager_Impl::preparePlan - "
               "Plan with UUID %s was successfully prepared.\n"));
  CORBA::Object_var ref = this->poa_->id_to_reference (id.in());
  return Deployment::DomainApplicationManager::_narrow (ref.in ());
}

::Deployment::DomainApplicationManagers *
ExecutionManager_Impl::getManagers ()
{
  DANCE_TRACE (DLINFO "ExecutionManager_Impl::getManagers ()");
  
  ::Deployment::DomainApplicationManagers * managers;
  ACE_NEW_THROW_EX (managers,
                    ::Deployment::DomainApplicationManagers(),
                    CORBA::NO_MEMORY());

  managers->length (this->managers_.current_size());
  unsigned int index = 0;
  for (TDomainManagers::iterator iter = this->managers_.begin();
       iter != this->managers_.end();
       ++iter)
    {
      CORBA::Object_var ref = this->poa_->servant_to_reference ( (*iter).int_id_);
      (*managers) [index]
      = Deployment::DomainApplicationManager::_narrow (ref.in ());
      index++;
    }

  return managers;
}

void
ExecutionManager_Impl::destroyManager (::Deployment::DomainApplicationManager_ptr appManager)
{
  DANCE_DEBUG((LM_DEBUG, DLINFO "ExecutionManager_Impl::destroyManager - started\n"));

  for (TDomainManagers::iterator iter = this->managers_.begin();
       iter != this->managers_.end();
       ++iter)
    {
      CORBA::Object_var app = this->poa_->servant_to_reference ( (*iter).int_id_);
      if (appManager->_is_equivalent (app.in ()))
        {
          this->managers_.unbind ( (*iter).ext_id_);
          PortableServer::ObjectId_var id = this->poa_->reference_to_id (appManager);
          this->poa_->deactivate_object (id.in());
          DANCE_DEBUG((LM_INFO, DLINFO "ExecutionManager_Impl::destroyManager - deleting DomainApplicationManager\n"));
          delete (*iter).int_id_;
          (*iter).int_id_ = 0;
          DANCE_DEBUG((LM_INFO, DLINFO "ExecutionManager_Impl::destroyManager - DomainApplicationManager deleted\n"));
          //this->managers_.unbind ( (*iter).ext_id_);
          DANCE_DEBUG((LM_INFO, DLINFO "ExecutionManager_Impl::destroyManager - finished\n"));
          return;
        }
    }
  DANCE_ERROR ((LM_ERROR, DLINFO "ExecutionManager_Impl::destroyManager - "
                 "corresponding DomainApplicationManager cannot be found\n"));
  throw ::Deployment::StopError();
}


