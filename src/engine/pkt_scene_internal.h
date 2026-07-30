#ifndef PKT_SCENE_INTERNAL_H
#define PKT_SCENE_INTERNAL_H

void __pkt_scene_update(float dt);
void __pkt_scene_draw(void);
int __pkt_scene_register(int scene_id, const struct pkt_scene *scene);
int __pkt_scene_swap(int next_scene_id);
int __pkt_scene_get(void);

#endif
