set(COMPONENT_SRCS "netconf.c"
				)
set(COMPONENT_ADD_INCLUDEDIRS ".")

set(COMPONENT_PRIV_REQUIRES esp_http_server esp_netif nvs_flash)
set(COMPONENT_EMBED_TXTFILES "root.html"
                             )

register_component()