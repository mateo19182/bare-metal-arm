sistema multifío de produtores e consumidores, que se comuniquen mediante unha cola de mensaxes
- os produtores van poñendo datos (empregaremos datos arbitrarios) na cola de mensaxes, empregando un pequeno delay para simular que lle leva un tempo a tarefa de obter cada dato.
- os consumidores van recollendo cada dato da cola, simulando tamén un procesamento do mesmo cun pequeno delay.

No LCD móstrase, nos dous díxitos da esquerda o número actual de produtores e de consumidores (un díxito para mostrar cada un destes números), mentres que os dous díxitos da dereita mostran a cantidade de datos pendentes na cola de mensaxes (>=0): cada vez que un produtor introduce un novo dato, aumenta (+1) esa conta, que diminue (-1) cada consumidor cando recolle un elemento para procesalo.

A nosa implementación empregará FreeRTOS. Cos botóns poderás cambiar o número de produtores e consumidores: o botón esquerdo conmutará entre 0-1-2-3-4 produtores, e o botón dereito entre 0-1-2-3-4 consumidores.Todos os fíos terán a mesma prioridade.
