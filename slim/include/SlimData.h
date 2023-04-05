#pragma once

#include <SlimType.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct SlimVector* SlimVector;
SlimVector slim_vector_create(u32_t element_size);
void slim_vector_destroy(SlimVector vector, void(*destructor)(void*));
void slim_vector_append(SlimVector vector, void* element);
void slim_vector_insert(SlimVector vector, u32_t index, void* element);
void slim_vector_remove(SlimVector vector, u32_t index, void* element);
void slim_vector_access(SlimVector vector, u32_t index, void* element);
u32_t slim_vector_size(SlimVector vector);

// TODO: Implement me and then use for bytecode loading (remove some reallocs and dynamic shenanigans)
void slim_vector_resize(SlimVector vector, u32_t size);
