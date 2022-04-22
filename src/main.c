#include "mgos.h"
#include "mgos_rpc.h"
#include "mgos_mqtt.h"

static void info_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                    struct mg_rpc_frame_info *fi, struct mg_str args)
{
  mg_rpc_send_responsef(ri,
                        "{typeCode: %d, version: %lf, deviceId: %Q}",
                        0,
                        1.0,
                        mgos_sys_config_get_cloudhome_device_id());
}

static void init_cb(struct mg_rpc_request_info *ri, void *cb_arg,
                    struct mg_rpc_frame_info *fi, struct mg_str args)
{
  char *uid;

  if (json_scanf(args.p, args.len, "{uid: %Q}", &uid) == 1)
  {
    const char *device_id = mgos_sys_config_get_cloudhome_device_id();

    LOG(LL_INFO, ("UID: %s", uid));

    char *topic = (char *)malloc(strlen("devices/") + strlen(device_id) + strlen("/init") + 1);
    sprintf(topic, "devices/%s/init", device_id);

    mgos_mqtt_pubf(topic, 1, false,
                   "{uid: %Q}", uid);

    while (mgos_mqtt_num_unsent_bytes() > 0)
    {
      mgos_msleep(100);
    }

    mg_rpc_send_responsef(ri, "{success: true}");
    return;
  }

  mg_rpc_send_responsef(ri, "{success: false}");
}

enum mgos_app_init_result mgos_app_init(void)
{
  mg_rpc_add_handler(mgos_rpc_get_global(), "Info.Get", "{}", info_cb, NULL);
  mg_rpc_add_handler(mgos_rpc_get_global(), "Init.Uid", "{uid: %Q}",
                     init_cb, NULL);
  // mgos_mqtt_pub("devices/test/init", "{uid: test}", 1, false);

  return MGOS_APP_INIT_SUCCESS;
}
