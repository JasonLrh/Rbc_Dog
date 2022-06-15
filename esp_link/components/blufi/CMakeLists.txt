idf_component_register(
        SRCS
            blufi_init.c  blufi_security.c  blufi_task.c  wifi_conf.c
        INCLUDE_DIRS
            "Inc"
        REQUIRES
            bt tcp-server uart-st
)