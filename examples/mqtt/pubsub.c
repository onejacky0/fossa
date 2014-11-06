/*
 * Copyright (c) 2014 Cesanta Software Limited
 * All rights reserved
 * This software is dual-licensed: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. For the terms of this
 * license, see <http://www.gnu.org/licenses/>.
 *
 * You are free to use this software under the terms of the GNU General
 * Public License, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * Alternatively, you can license this software under a commercial
 * license, as set out in <http://cesanta.com/>.
 */

#include "fossa.h"

struct ns_mqtt_topic_expression topic_expressions[] = {
  {"/stuff", 0}
};

static void ev_handler(struct ns_connection *nc, int ev, void *p) {
  struct ns_mqtt_message *msg = (struct ns_mqtt_message *)p;
  (void) nc;

#if 0
  if (ev != NS_POLL)
    printf("USER HANDLER GOT %d\n", ev);
#endif

  switch (ev) {
    case NS_CONNECT:
      ns_set_protocol_mqtt(nc);
      ns_send_mqtt_handshake(nc, "dummy");
      break;
    case NS_MQTT_CONNACK:
      if (msg->connack_ret_code != NS_MQTT_CONNACK_ACCEPTED) {
        printf("Got mqtt connection error: %d\n", msg->connack_ret_code);
        exit(1);
      }
      printf("Subscribing to '/stuff'\n");
      ns_mqtt_subscribe(nc, topic_expressions, sizeof(topic_expressions)/sizeof(struct ns_mqtt_topic_expression), 42);
      break;
    case NS_MQTT_PUBACK:
      printf("Message publishing acknowledged (msg_id: %d)\n", msg->message_id);
      break;
    case NS_MQTT_SUBACK:
      printf("Subscription acknowledged, forwarding to '/test'\n");
      break;
    case NS_MQTT_PUBLISH:
      {
#if 0
        char hex[1024] = {0};
        ns_hexdump(nc->recv_iobuf.buf, msg->payload_len, hex, sizeof(hex));
        printf("Got incoming message %s:\n%s", msg->topic, hex);
#else
        printf("Got incoming message %s: %.*s\n", msg->topic, msg->payload_len, nc->recv_iobuf.buf);
#endif

        printf("Forwarding to /test\n");
        ns_mqtt_publish(nc, "/test", 65, NS_MQTT_QOS(0), nc->recv_iobuf.buf, msg->payload_len);
      }
      break;
    case NS_CLOSE:
      printf("Connection closed\n");
      exit(1);
  }
}

int main(int argc, char *argv[]) {
  struct ns_mgr mgr;
  const char *address = "localhost:1883";
  (void)argc;
  (void)argv;

  ns_mgr_init(&mgr, NULL);

  if (ns_connect(&mgr, address, ev_handler) == NULL) {
    fprintf(stderr, "ns_connect(%s) failed\n", address);
    exit(EXIT_FAILURE);
  }

  for(;;) {
    ns_mgr_poll(&mgr, 1000);
  }
}