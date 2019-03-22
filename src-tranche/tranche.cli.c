

int main(int argc, char **argv, char **envp){

  if(argc != 2){
    fprintf(stderr, "usage: %s <source-file>\n",argv[0]);
  }
  FILE *file = fopen(argv[1], "r");

  Da targets;
  da_alloc(&targets, sizeof(int));
  da_push(&da_targets, stdout);
  tranche_send(file, &da_targets);
  da_free(targets);
  fclose(file);

}
