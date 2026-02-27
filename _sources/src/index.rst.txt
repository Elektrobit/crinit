Crinit Source
=============

Setting capabilities on crinit tasks
------------------------------------
Crinit tasks can be configured to be equipped with a specific set of capabilities. Technically, this is enforced by the helper program `crinit-launch`. As `crinit-launch` is executed only if a crinit task is configured to be run as a non-privileged user, capabilities are configured only in that case consequently.

Below diagram shall depict the steps (amongst other that are conveniently not shown) that are taken by `crinit-launch` in order to apply configured capabilities on a task.

.. figure:: /doc/diagrams/capability_activity.png
