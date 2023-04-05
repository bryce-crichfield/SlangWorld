#include <SlimData.h>

struct SlimVector {
    u32_t size;
    u32_t capacity;
    u32_t element_size;
    u8_t* data;
};

SlimVector slim_vector_create(u32_t element_size)
{
    SlimVector vector = malloc(sizeof(SlimVector));
    vector->size = 0;
    vector->capacity = 0;
    vector->element_size = element_size;
    vector->data = NULL;
    return vector;
}

void slim_vector_destroy(SlimVector vector, void (*destructor)(void*))
{
    assert(vector != NULL);

    if (destructor != NULL) {
        for (u32_t i = 0; i < vector->size; i++) {
            destructor(vector->data + i * vector->element_size);
        }
    }


    if (vector->data != NULL) {
        printf("freeing vector data\n");
        free(vector->data);
    }
    
    printf("freeing vector\n");

    free(vector);
    vector = NULL;
}

void slim_vector_append(SlimVector vector, void* element)
{
    assert(vector != NULL);
    assert(element != NULL);
    slim_vector_insert(vector, vector->size, element);
}

// Copies the data from the element into the vector
void slim_vector_insert(SlimVector vector, u32_t index, void* element)
{
    assert(vector != NULL);
    assert(element != NULL);
    assert(index <= vector->size);
    if (vector->size == vector->capacity) {
        vector->capacity = vector->capacity == 0 ? 1 : vector->capacity * 2;
        vector->data = realloc(vector->data, vector->capacity * vector->element_size);
    }
    if (index < vector->size) {
        memmove(vector->data + (index + 1) * vector->element_size, vector->data + index * vector->element_size,
            (vector->size - index) * vector->element_size);
    }
    memcpy(vector->data + index * vector->element_size, element, vector->element_size);
    vector->size++;
}

// Copies the data from the vector into the element, and removes the element from the vector
void slim_vector_remove(SlimVector vector, u32_t index, void* element)
{
    assert(vector != NULL);
    assert(element != NULL);
    assert(index < vector->size);
    memcpy(element, vector->data + index * vector->element_size, vector->element_size);
    if (index < vector->size - 1) {
        memmove(vector->data + index * vector->element_size, vector->data + (index + 1) * vector->element_size,
            (vector->size - index - 1) * vector->element_size);
    }
    vector->size--;
}

// Copies the vector data to the element location
void slim_vector_access(SlimVector vector, u32_t index, void* element)
{
    assert(vector != NULL);
    assert(element != NULL);
    assert(index < vector->size);
    memcpy(element, vector->data + index * vector->element_size, vector->element_size);
}

u32_t slim_vector_size(SlimVector vector)
{
    assert(vector != NULL);
    return vector->size;
}