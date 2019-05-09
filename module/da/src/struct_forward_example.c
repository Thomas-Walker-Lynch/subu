

typedef struct AccChannel_struct AccChannel;
typedef struct Da_struct Da;

typedef long long int size_t;

struct AccChannel_struct{
  struct Da *outstanding_malloc;
  Da *spurious_free;
  //  enum Mode mode;
}; //name instances of channels with handles

struct Da_struct{
  char *base;
  char *end; // one byte/one element off the end of the array
  size_t size; // size >= (end - base) + 1;
  size_t element_size;
  AccChannel *channel;//assign during init, set to NULL during free
};


int main(){
  Da x;
  AccChannel y;
  return 5;
}
