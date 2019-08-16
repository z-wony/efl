#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#define EFL_UI_WIDGET_PROTECTED
#define EFL_PART_PROTECTED

#include <Efl_Ui.h>
#include "elm_priv.h"
#include "efl_ui_property_bind_part.eo.h"

typedef struct _Efl_Ui_Widget_Factory_Data Efl_Ui_Widget_Factory_Data;
typedef struct _Efl_Ui_Widget_Factory_Request Efl_Ui_Widget_Factory_Request;
typedef struct _Efl_Ui_Bind_Part_Data Efl_Ui_Bind_Part_Data;
typedef struct _Efl_Ui_Property_Bind_Data Efl_Ui_Property_Bind_Data;

struct _Efl_Ui_Property_Bind_Data
{
   Eina_Stringshare *part_property;
   Eina_Stringshare *model_property;
};

struct _Efl_Ui_Bind_Part_Data
{
   Eina_Stringshare *part;

   Eina_List *properties;
};

struct _Efl_Ui_Widget_Factory_Data
{
   const Efl_Class *klass;

   Eina_Hash *parts;

   Eina_Stringshare *style;
};

struct _Efl_Ui_Widget_Factory_Request
{
   Efl_Ui_Widget_Factory_Data *pd;
   Eo *parent;
};

static void
_efl_ui_widget_factory_item_class_set(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                      const Efl_Class *klass)
{
   if (!efl_isa(klass, EFL_UI_VIEW_INTERFACE) ||
       !efl_isa(klass, EFL_UI_WIDGET_CLASS))
     {
        ERR("Provided class '%s' for factory '%s' doesn't implement '%s' and '%s' interfaces.",
            efl_class_name_get(klass),
            efl_class_name_get(obj),
            efl_class_name_get(EFL_UI_WIDGET_CLASS),
            efl_class_name_get(EFL_UI_VIEW_INTERFACE));
        return ;
     }
   pd->klass = klass;
}

static const Efl_Class *
_efl_ui_widget_factory_item_class_get(const Eo *obj EINA_UNUSED,
                                      Efl_Ui_Widget_Factory_Data *pd)
{
   return pd->klass;
}

static Efl_Ui_Widget *
_efl_ui_widget_create(const Efl_Class *klass, Eo *parent,
                      const char *style, Efl_Model *model,
                      const Eina_Hash *parts)
{
   Efl_Ui_Bind_Part_Data *bpd;
   Eina_Iterator *it;
   Efl_Ui_Widget *w;

   w = efl_add(klass, parent,
               style ? efl_ui_widget_style_set(efl_added, style) : (void) 0,
               efl_ui_view_model_set(efl_added, model));
   if (!parts) return w;

   it = eina_hash_iterator_data_new(parts);
   EINA_ITERATOR_FOREACH(it, bpd)
     {
        Efl_Ui_Property_Bind_Data *bppd;
        Eina_List *l;

        EINA_LIST_FOREACH(bpd->properties, l, bppd)
          efl_ui_property_bind(efl_part(w, bpd->part),
                               bppd->part_property,
                               bppd->model_property);
     }
   eina_iterator_free(it);

   return w;
}

static Eina_Value
_efl_ui_widget_factory_create_then(Eo *model, void *data, const Eina_Value v)
{
   Efl_Ui_Widget_Factory_Request *r = data;
   Efl_Ui_Widget *w;
   const char *string = NULL;

   if (!eina_value_string_get(&v, &string))
     return eina_value_error_init(EFL_MODEL_ERROR_NOT_SUPPORTED);

   w = _efl_ui_widget_create(r->pd->klass, r->parent, string, model, r->pd->parts);
   if (!w) return eina_value_error_init(ENOMEM);
   return eina_value_object_init(w);
}

static void
_efl_ui_widget_factory_single_cleanup(Eo *model, void *data EINA_UNUSED, const Eina_Future *dead_future EINA_UNUSED)
{
   efl_unref(model);
}

static void
_efl_ui_widget_factory_create_cleanup(Eo *o EINA_UNUSED, void *data, const Eina_Future *dead_future EINA_UNUSED)
{
   Efl_Ui_Widget_Factory_Request *r = data;

   efl_unref(r->parent);
   free(r);
}

static Eina_Future *
_efl_ui_widget_factory_efl_ui_factory_create(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                             Eina_Iterator *models, Efl_Gfx_Entity *parent)
{
   Efl_Ui_Widget_Factory_Request *r;
   Eina_Future **f;
   Efl_Model *model;
   int count = 0;

   if (!pd->klass)
     return efl_loop_future_rejected(obj, EFL_MODEL_ERROR_INCORRECT_VALUE);

   if (!pd->style)
     {
        Efl_Ui_Widget *w;
        Eina_Value r;

        eina_value_array_setup(&r, EINA_VALUE_TYPE_OBJECT, 4);

        EINA_ITERATOR_FOREACH(models, model)
          {
             w = _efl_ui_widget_create(pd->klass, parent, NULL, model, pd->parts);

             if (!w) return efl_loop_future_rejected(obj, ENOMEM);
             eina_value_array_append(&r, w);
          }
        eina_iterator_free(models);

        return efl_loop_future_resolved(obj, r);
     }

   r = calloc(1, sizeof (Efl_Ui_Widget_Factory_Request));
   if (!r) return efl_loop_future_rejected(obj, ENOMEM);

   r->pd = pd;
   r->parent = efl_ref(parent);

   f = calloc(count + 1, sizeof (Eina_Future *));
   if (!f) return efl_loop_future_rejected(obj, ENOMEM);

   EINA_ITERATOR_FOREACH(models, model)
     {
        f[count++] = efl_future_then(efl_ref(model), efl_model_property_ready_get(model, pd->style),
                                     .success = _efl_ui_widget_factory_create_then,
                                     .free = _efl_ui_widget_factory_single_cleanup);

        f = realloc(f, (count + 1) * sizeof (Eina_Future *));
        if (!f) return efl_loop_future_rejected(obj, ENOMEM);
     }
   eina_iterator_free(models);

   f[count] = EINA_FUTURE_SENTINEL;

   return efl_future_then(obj, eina_future_all_array(f),
                          .data = r,
                          .free = _efl_ui_widget_factory_create_cleanup);
}

static void
_efl_ui_widget_factory_efl_ui_factory_release(Eo *obj EINA_UNUSED,
                                              Efl_Ui_Widget_Factory_Data *pd EINA_UNUSED,
                                              Efl_Gfx_Entity *ui_view)
{
   // We do not cache or track this item, just get rid of them asap
   efl_del(ui_view);
}

Eina_Stringshare *_property_style_ss = NULL;

static Eina_Error
_efl_ui_widget_factory_efl_ui_property_bind_property_bind(Eo *obj, Efl_Ui_Widget_Factory_Data *pd,
                                                          const char *target, const char *property)
{
   if (_property_style_ss == target || !strcmp(target, _property_style_ss))
     {
        eina_stringshare_replace(&pd->style, property);
        efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) _property_style_ss);
        return 0;
     }

   return EINVAL;
}


typedef struct _Efl_Ui_Property_Bind_Part_Data Efl_Ui_Property_Bind_Part_Data;
struct _Efl_Ui_Property_Bind_Part_Data
{
   Efl_Ui_Widget_Factory_Data *pd;
   Eina_Stringshare *name;
};

static Efl_Object *
_efl_ui_widget_factory_efl_part_part_get(const Eo *obj,
                                         Efl_Ui_Widget_Factory_Data *pd,
                                         const char *name)
{
   Efl_Ui_Property_Bind_Part_Data *ppd;
   Efl_Object *part;

   part = efl_add(EFL_UI_PROPERTY_BIND_PART_CLASS, (Eo*) obj);
   if (!part) return NULL;

   ppd = efl_data_scope_get(part, EFL_UI_PROPERTY_BIND_PART_CLASS);
   ppd->name = eina_stringshare_add(name);
   ppd->pd = pd;

   return part;
}

static void
_efl_ui_property_bind_part_efl_object_destructor(Eo *obj, Efl_Ui_Property_Bind_Part_Data *pd)
{
   eina_stringshare_replace(&pd->name, NULL);

   efl_destructor(efl_super(obj, EFL_UI_PROPERTY_BIND_PART_CLASS));
}

static Eina_Error
_efl_ui_property_bind_part_efl_ui_property_bind_property_bind(Eo *obj EINA_UNUSED,
                                                              Efl_Ui_Property_Bind_Part_Data *pd,
                                                              const char *key,
                                                              const char *property)
{
   Efl_Ui_Bind_Part_Data *bpd;
   Efl_Ui_Property_Bind_Data *bppd;

   if (!pd->pd)
     {
        EINA_LOG_ERR("Trying to bind part property without specifying which part");
        return ENOENT;
     }

   if (!pd->pd->parts)
     pd->pd->parts = eina_hash_stringshared_new(NULL);

   bpd = eina_hash_find(pd->pd->parts, pd->name);
   if (!bpd)
     {
        bpd = calloc(1, sizeof (Efl_Ui_Bind_Part_Data));
        if (!bpd) return ENOMEM;

        bpd->part = eina_stringshare_ref(pd->name);

        eina_hash_direct_add(pd->pd->parts, bpd->part, bpd);
     }

   bppd = calloc(1, sizeof (Efl_Ui_Property_Bind_Data));
   if (!bppd) return ENOMEM;

   bppd->part_property = eina_stringshare_add(key);
   bppd->model_property = eina_stringshare_add(property);

   bpd->properties = eina_list_append(bpd->properties, bppd);

   efl_event_callback_call(obj, EFL_UI_PROPERTY_BIND_EVENT_PROPERTY_BOUND, (void*) key);

   return 0;
}

#include "efl_ui_property_bind_part.eo.c"
#include "efl_ui_widget_factory.eo.c"
