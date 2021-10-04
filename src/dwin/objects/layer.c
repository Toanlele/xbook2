#include <dwin/objects.h>
#include <dwin/dwin.h>
#include <dwin/layer.h>
#include <dwin/workstation.h>

dwin_layer_t *dwin_layer_create(uint32_t width, uint32_t height)
{
    dwin_layer_t *layer = dwin_malloc(sizeof(dwin_layer_t));
    if (layer == NULL)
    {
        return NULL;
    }
    layer->buffer = dwin_malloc(DWIN_LAYER_BUF_SZ(width, height));
    if (layer->buffer == NULL)
    {
        dwin_free(layer);
        return NULL;
    }

    layer->x = -1;
    layer->y = -1;
    layer->z = -1;

    layer->width = width;
    layer->height = height;
    layer->workstation = NULL;

    layer->id = dwin_layer_alloc_id();

    if (layer->id == -1)
    {
        dwin_free(layer->buffer);
        dwin_free(layer);
        return NULL;
    }

    list_init(&layer->list);
    list_init(&layer->global_list);

    return layer;
}

/**
 * NOTICE: layer should not on the workstatin
 */
int dwin_layer_destroy(dwin_layer_t *layer)
{
    if (layer == NULL)
    {
        return -1;
    }
    
    list_del_init(&layer->list);
    list_del_init(&layer->global_list);

    dwin_layer_free_id(layer->id);
    layer->id = -1;
    
    dwin_free(layer->buffer);
    layer->buffer = NULL;

    dwin_free(layer);
    layer = NULL;
    return 0;
}

/**
 * delete a layer on the workstation 
 */
int dwin_layer_delete(dwin_layer_t *layer)
{
    if (layer == NULL)
    {
        return -1;
    }

    if (layer->workstation == NULL)
    {
        return -1;
    }
    
    /* NOTE: hide layer first */
    dwin_layer_zorder(layer, -1);

    if (dwin_workstation_del_layer(layer->workstation, layer) < 0)
    {
        return -1;
    }

    return dwin_layer_destroy(layer);
}

void dwin_layer_test(void)
{
    dwin_layer_t *ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    dwin_assert(dwin_layer_delete(ly) == 0);

    dwin_layer_t *ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_workstation_get_by_depth(0), ly2);
    dwin_assert(dwin_layer_delete(ly2) == 0);

    ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    dwin_assert(dwin_layer_delete(ly) == 0);

    ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_workstation_get_by_depth(0), ly2);
    dwin_assert(dwin_layer_delete(ly2) == 0);

    ly = dwin_layer_create(dwin_current_workstation->width, dwin_current_workstation->height);
    dwin_assert(ly != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly);
    
    memset(ly->buffer, 0xff, ly->width * ly->height * DWIN_LAYER_BPP);
    dwin_layer_zorder(ly, 0);

    ly2 = dwin_layer_create(dwin_current_workstation->width / 2, dwin_current_workstation->height / 2);
    dwin_assert(ly2 != NULL);
    dwin_workstation_add_layer(dwin_current_workstation, ly2);
    
    memset(ly2->buffer, 0x5a, ly2->width * ly2->height * DWIN_LAYER_BPP);
    dwin_layer_zorder(ly2, 0);
    dwin_layer_move(ly2, 100, 50);

    dwin_layer_resize(ly2, 200, 200, 50, 100);
    memset(ly2->buffer, 0x5a, ly2->width * ly2->height * DWIN_LAYER_BPP);

    dwin_layer_flush(ly2, 0, 0, ly2->width, ly2->height);

    dwin_assert(dwin_layer_delete(ly) == 0);
    dwin_assert(dwin_layer_delete(ly2) == 0);
}

void dwin_layer_init(void)
{
    dwin_layer_id_init();
    dwin_layer_test();
}
