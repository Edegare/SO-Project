# Projeto de Sistemas Operativos

Pretende-se implementar um serviço de orquestração (i.e., execução e escalonamento) de tarefas num computador. 

Os utilizadores devem usar um programa cliente para submeter ao servidor a intenção de executar uma tarefa, dando uma indicação da duração em milissegundos que necessitam para a mesma, e qual a tarefa (i.e., programa ou conjunto/pipeline de programas) a executar. 

Cada tarefa tem associado um identificador único que deve ser transmitido ao cliente mal o servidor recebe a mesma. O servidor é responsável por escalonar e executar as tarefas dos utilizadores. Informação produzida pelas tarefas para o standard output ou standard error devem ser redirecionadas pelo servidor para um ficheiro cujo nome corresponde ao identificador da tarefa. 

Através do programa cliente, os utilizadores podem ainda consultar o servidor para saberem quais as tarefas em execução, em espera para execução, e terminadas.

## Instalação

Dentro do seu repositório, use para compilar: 

    make


### Inicialização do Servidor: 

    ./bin/orchestrator output_folder parallel-tasks sched-policy

Argumentos:
1. output-folder: pasta onde são guardados os ficheiros com o output de tarefas executadas.
2. parallel-tasks: número de tarefas que podem ser executadas em paralelo.
3. sched-policy: identificador da política de escalonamento, caso o servidor suporte várias políticas.

Exemplo:

    ./bin/orchestrator tmp 5 0 -- Máximo de 5 tarefas a executar concorrentemente

### Inicialização dos clientes: 
    
Para verificar estado do servidor:

    ./bin/client status

Para executar programa (-u = single ; -p = pipeline):

    ./bin/client execute time -option "progs"

Uso de script para criar clientes e suas tarefas:

    chmod +x tests/client_script.sh
    ./tests/client_script.sh

