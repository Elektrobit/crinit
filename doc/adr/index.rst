Architecture Design Records
===========================

During the development of crinit, many decisions have to be made. This chapter
provides a place to collect new ideas and discuss them until we are able to
agree on a final decision of an topic.


An *Architecture Design Record* (**ADR**) should follow roughly the pattern

* Problem Description
* Influencing factors
* Assumptions
* Considered Alternatives
* Decision
   * Rationale
   * Open Points

There is an :doc:`ADR template<ADR_template>` which can be used as a starting
point.


List of ADRs
------------

.. toctree::
   :maxdepth: 1
   :caption: Contents:

   ADR Template<./ADR_template.md>
   ./adr-capability-library.md
   ./adr-capability-setting.md
   ./adr-crypt-lib.md
   ./adr-pipe-redirection.md
   ./adr-pubkey-storage.md
   ./adr-secondary-init.md
   ./adr-signature-algorithm.md
   ./adr-signature-conf-interface.md
   ./adr-signature-storage.md
   ./adr-task-include-strategy.md
   ./adr-cgroups-assigning-processes.md
   ./adr-cgroups-hierachic-cgroups.md
   ./adr-cgroups-invalid-configuration.md
   ./adr-cgroups-management.md
   ./adr-cgroups-where-to-define-cgroups.md
