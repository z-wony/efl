#ifndef SIMPLE_H
#define SIMPLE_H

typedef struct
{
   int a;
} Simple_Public_Data;

EAPI void simple_a_set(Eo *obj, int a);
EAPI int simple_a_get(Eo *obj);
EAPI Eina_Bool simple_a_print(Eo *obj);
EAPI Eina_Bool simple_class_hi_print(const Eo *obj);
EAPI void simple_recursive(Eo *obj, int n);
EAPI void simple_pure_virtual(Eo *obj);
EAPI void simple_no_implementation(Eo *obj);
EAPI Eo *simple_part_get(Eo *obj, const char *name);

extern const Eo_Event_Description _EV_A_CHANGED;
#define EV_A_CHANGED (&(_EV_A_CHANGED))

extern const Eo_Event_Description _EV_A_CHANGED2;
#define EV_A_CHANGED2 (&(_EV_A_CHANGED2))

#define SIMPLE_CLASS simple_class_get()
const Eo_Class *simple_class_get(void);

#endif
