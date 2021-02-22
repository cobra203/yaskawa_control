#ifndef _SV660C_NODE_H_
#define _SV660C_NODE_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include <mod_gauguin.h>

CBA_BOOL sv660c_node_register(CAN_NODE_S *node);
CBA_BOOL sv660c_node_release(CAN_NODE_S *node);
CBA_BOOL sv660c_node_get_identity(CAN_NODE_S *node);

#ifdef __cplusplus
}
#endif

#endif /* _SV660C_NODE_H_ */
