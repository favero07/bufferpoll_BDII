#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "buffend.c"

int menu();

int buscar_tabela(char tabela[], char busca[]){
	int erro;
	
	struct fs_objects objeto = leObjeto(tabela);

	tp_table *esquema = leSchema(objeto);

	if(esquema == ERRO_ABRIR_ESQUEMA){
		printf("Erro ao criar o esquema.\n");
		return 0;
	}

	tp_buffer *bufferpoll = initbuffer();

	if(bufferpoll == ERRO_DE_ALOCACAO){
		printf("Erro ao alocar memória para o buffer.\n");
		return 0;
	}

	erro = colocaTuplaBuffer(bufferpoll, 0, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}
	
	erro = colocaTuplaBuffer(bufferpoll, 1, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}
	
	erro = colocaTuplaBuffer(bufferpoll, 2, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}

	column *pagina = getPage(bufferpoll, esquema, objeto, 0);

	if(pagina == ERRO_PARAMETRO){
		printf("Erro, na função getPage(), problemas no parametro.\n");
		return 0;
	}

	// Impressão dos dados da tupla buscada
	int j=0,y=0,aux=0;

	for(j=0; j < objeto.qtdCampos*bufferpoll[0].nrec; j++){
		//Caso for string, compara com o campo buscado
		if(pagina[j].tipoCampo == 'S'){
			if (strcmp (pagina[j].valorCampo, busca) == 0){
				aux = 1;
				printf("Tupla encontrada! Os dados serão mostrados abaixo:\n\n");
				for(y=j;y<j+4;y++){
					if(pagina[y].tipoCampo == 'S')
						printf("%s: %s ", pagina[y].nomeCampo,pagina[y].valorCampo);
					else if(pagina[y].tipoCampo == 'I'){
						int *n = (int *)&pagina[y].valorCampo[0];
						printf("%s: %d ",pagina[y].nomeCampo, *n);
					}
					else if(pagina[y].tipoCampo == 'C'){
						printf("%s: %c ",pagina[y].nomeCampo, pagina[y].valorCampo[0]);
					}
					else if(pagina[y].tipoCampo == 'D'){
						double *n = (double *)&pagina[y].valorCampo[0];
						printf("%s: %f ",pagina[y].nomeCampo, *n);
					}
					printf("\n");
				}
				printf("\n");
			}
		}
	}

	//Verifica se encontrou nome buscado
	if(aux==0)
		printf("'%s' não foi encontrado na tabela %s!\n",busca,tabela);
	
	//~ erro = printbufferpoll(bufferpoll, esquema, objeto, 0);

	//~ if(erro != SUCCESS){
		//~ printf("Erro %d: na função printbufferpoll().\n", erro);
		//~ return 0;
	//~ }

	return 0;
}

int consultar(){
	char tabela[TAMANHO_NOME_TABELA];
	char busca[20];

	printf("\nDeseja consultar os dados de qual tabela? (Tabelas Padroes: Cliente, Funcionario, Produto, Cargo)\n");
	scanf("%s",tabela);
	
	printf("\nVoce selecionou Consultar!\nDigite o nome/descricao que deseja consultar:\n");
	scanf(" %s",busca);

	printf("\nVoce esta buscando por %s em %s\n",busca,tabela);
	buscar_tabela(tabela,busca);

	return 1;
}

//tipo_tabela utilizado apenas enquanto dados forem criados de forma estatica
int criar_tabela(char nomeTabela[], int tipo_tabela){
	int erro;
	//~ char nomeTabela[TAMANHO_NOME_TABELA];

	//~ printf("Qual o nome da tabela que deseja criar?\n");
	//~ scanf("%s",nomeTabela);

	table *t = NULL;
	column *c = NULL;

	t = iniciaTabela(nomeTabela);

	if(t == ERRO_NOME_TABELA_INVALIDO){
		printf("Erro: na função iniciaTabela(). Nome da tabela já existente.\n");
		return 0;
	}

	switch(tipo_tabela){
		
		case 1://Clientes
			t = adicionaCampo(t, "Nome", 'S', 20);
			t = adicionaCampo(t, "Idade", 'I', (sizeof(int)));
			t = adicionaCampo(t, "Sexo", 'C', (sizeof(char)));
			t = adicionaCampo(t, "Obs", 'S', 40);
			t = adicionaCampo(t, "Valor Gasto", 'D', (sizeof(double)));
		break;
		
		case 2://Funcionarios
			t = adicionaCampo(t, "Nome", 'S', 20);
			t = adicionaCampo(t, "Idade", 'I', (sizeof(int)));
			t = adicionaCampo(t, "Sexo", 'C', (sizeof(char)));
			t = adicionaCampo(t, "Obs", 'S', 40);
			t = adicionaCampo(t, "Salario", 'D', (sizeof(double)));
		break;
		
		case 3://Produtos
			t = adicionaCampo(t, "Produto", 'S', 20);
			t = adicionaCampo(t, "Ano Validade", 'I', (sizeof(int)));
			t = adicionaCampo(t, "Novo", 'C', (sizeof(char)));
			t = adicionaCampo(t, "Descricao", 'S', 40);
			t = adicionaCampo(t, "Preco", 'D', (sizeof(double)));
		break;
		
		case 4://Cargos
			t = adicionaCampo(t, "Cargo", 'S', 20);
			t = adicionaCampo(t, "Vagas", 'I', (sizeof(int)));
			t = adicionaCampo(t, "Tipo de Contrato", 'C', (sizeof(char)));
			t = adicionaCampo(t, "Obs", 'S', 40);
			t = adicionaCampo(t, "Salario", 'D', (sizeof(double)));
		break;
	}

	erro = finalizaTabela(t);

	if(erro != SUCCESS){
		printf("Erro %d: na função finalizaTabela().\n", erro);
		return 0;
	}

	switch(tipo_tabela){

		case 1://Clientes
			c = insereValor(c, "Nome", "Joao");
			c = insereValor(c, "Idade", "40");
			c = insereValor(c, "Sexo", "F");
			c = insereValor(c, "Obs", "Obs. Um");
			c = insereValor(c, "Valor Gasto", "25.5");

			c = insereValor(c, "Nome", "Maria");
			c = insereValor(c, "Idade", "20");
			c = insereValor(c, "Sexo", "M");
			c = insereValor(c, "Obs", "Obs. Dois");
			c = insereValor(c, "Valor Gasto", "10.67");

			c = insereValor(c, "Nome", "Pedro");
			c = insereValor(c, "Idade", "30");
			c = insereValor(c, "Sexo", "F");
			c = insereValor(c, "Obs", "Obs. Tres");
			c = insereValor(c, "Valor Gasto", "14.56");
		break;
		
		case 2://Funcionarios
			c = insereValor(c, "Nome", "Func1");
			c = insereValor(c, "Idade", "40");
			c = insereValor(c, "Sexo", "F");
			c = insereValor(c, "Obs", "Obs. Um");
			c = insereValor(c, "Salario", "2500.00");

			c = insereValor(c, "Nome", "Func2");
			c = insereValor(c, "Idade", "20");
			c = insereValor(c, "Sexo", "M");
			c = insereValor(c, "Obs", "Obs. Dois");
			c = insereValor(c, "Salario", "1670.00");

			c = insereValor(c, "Nome", "Func3");
			c = insereValor(c, "Idade", "30");
			c = insereValor(c, "Sexo", "F");
			c = insereValor(c, "Obs", "Obs. Tres");
			c = insereValor(c, "Salario", "1450.60");
		break;
		
		case 3://Produtos
			c = insereValor(c, "Produto", "Produto1");
			c = insereValor(c, "Ano Validade", "2014");
			c = insereValor(c, "Novo", "N");
			c = insereValor(c, "Descricao", "Descr Um");
			c = insereValor(c, "Preco", "50.00");

			c = insereValor(c, "Produto", "Produto2");
			c = insereValor(c, "Ano Validade", "2015");
			c = insereValor(c, "Novo", "S");
			c = insereValor(c, "Descricao", "Descr Dois");
			c = insereValor(c, "Preco", "16.00");

			c = insereValor(c, "Produto", "Produto3");
			c = insereValor(c, "Ano Validade", "2017");
			c = insereValor(c, "Novo", "S");
			c = insereValor(c, "Descricao", "Descr Tres");
			c = insereValor(c, "Preco", "14.60");
		break;
		
		case 4://Cargos
			c = insereValor(c, "Cargo", "Cargo1");
			c = insereValor(c, "Vagas", "40");
			c = insereValor(c, "Tipo de Contrato", "F");
			c = insereValor(c, "Obs", "Obs. Um");
			c = insereValor(c, "Salario", "2500.00");

			c = insereValor(c, "Cargo", "Cargo2");
			c = insereValor(c, "Vagas", "20");
			c = insereValor(c, "Tipo de Contrato", "M");
			c = insereValor(c, "Obs", "Obs. Dois");
			c = insereValor(c, "Salario", "1500.00");

			c = insereValor(c, "Cargo", "Cargo3");
			c = insereValor(c, "Vagas", "30");
			c = insereValor(c, "Tipo de Contrato", "F");
			c = insereValor(c, "Obs", "Obs. Tres");
			c = insereValor(c, "Salario", "1000.00");
		break;
	}

	erro = finalizaInsert(nomeTabela, c);

	if(erro != SUCCESS){
		printf("Erro %d: na função finalizaInsert()\n", erro);
		return 0;
	}

	struct fs_objects objeto = leObjeto(nomeTabela);
	
	tp_table *esquema = leSchema(objeto);

	if(esquema == ERRO_ABRIR_ESQUEMA){
		printf("Erro ao criar o esquema.\n");
		return 0;
	}

	tp_buffer *bufferpoll = initbuffer();

	if(bufferpoll == ERRO_DE_ALOCACAO){
		printf("Erro ao alocar memória para o buffer.\n");
		return 0;
	}

	erro = colocaTuplaBuffer(bufferpoll, 0, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}
	
	erro = colocaTuplaBuffer(bufferpoll, 1, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}
	
	erro = colocaTuplaBuffer(bufferpoll, 2, esquema, objeto);

	if(erro != SUCCESS){
		printf("Erro %d: na função colocaTuplaBuffer().\n", erro);
		return 0;
	}
	
	column *tuplaE = excluirTuplaBuffer(bufferpoll, esquema, objeto, 0, 2); //pg, tupla
	column *pagina = getPage(bufferpoll, esquema, objeto, 0);

	if(tuplaE == ERRO_PARAMETRO){
		printf("Erro, na função excluirTuplaBuffer(), problemas no parametro.\n");
		return 0;
	}
	if(pagina == ERRO_PARAMETRO){
		printf("Erro, na função getPage(), problemas no parametro.\n");
		return 0;
	}

	printf("\nTabela %s criada com sucesso!\n",nomeTabela);
	
	return 1;
}

//Cria tabelas pre-estabelecidas
void criar_tabelas_padrao(){
	criar_tabela("Cliente",1);
	criar_tabela("Funcionario",2);
	criar_tabela("Produto",3);
	criar_tabela("Cargo",4);
}

//Verifica se o arquivo existe
int existe_arquivo(const char* nomeArquivo){
	FILE* arquivo = fopen(nomeArquivo, "r"); 
	if(arquivo!=NULL){
		fclose(arquivo);
		
		return 1;
	}
	return 0;
}

int excluir_arquivo(char* nomeTabela){
	char *nomeArquivo=(char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
	int i;
    //~ struct fs_objects objeto;

	strcpy(nomeArquivo,nomeTabela);
	strcat(nomeArquivo,".dat");

	//Verifica se arquivo existe
	if(existe_arquivo(nomeArquivo)){
		if(remove(nomeArquivo)){
			printf("Erro ao remover arquivo das tuplas!\n");
			return 0;
		}
	}
	
	//Verifica se tabela existe no dicionário
	if(!verificaNomeTabela(nomeTabela)){
		printf("Tabela nao existente!\n");
		return 0;
	}
	
	//~ objeto = leObjeto(nomeTabela);

	FILE *dicionario;
	char *tupla = (char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
	if((dicionario = fopen("fs_object.dat","a+b")) == NULL)
    	return ERRO_ABRIR_ARQUIVO;

	//~ FILE *dicionario_novo;
	//~ dicionario_novo = fopen("fs_object2.dat","wb");

	while(fgetc (dicionario) != EOF){
		fseek(dicionario, -1, 1);

		fread(tupla, sizeof(char), TAMANHO_NOME_TABELA, dicionario); //Lê somente o nome da tabela

		if(strcmp(tupla, nomeTabela) == 0){ // Verifica se o nome dado pelo usuario existe no dicionario de dados.
			fseek(dicionario, -TAMANHO_NOME_TABELA, 1); //Retorna para posição inicial da tabela

			//~ for(i=0;i<TAMANHO_NOME_TABELA+TAMANHO_NOME_ARQUIVO;i++){ // Enquanto estiver nas posiçoes da tabela
				//~ printf("%c",fgetc (dicionario));
				//~ fwrite("\0",1,1,dicionario_novo);
			//~ }
		}else{
			fseek(dicionario, -TAMANHO_NOME_TABELA, 1);
			for(i=0;i<TAMANHO_NOME_TABELA+TAMANHO_NOME_ARQUIVO;i++){ // Enquanto estiver nas posiçoes da tabela
				//~ fwrite(fgetc (dicionario),1,1,dicionario_novo);
			}
		}
		
        fseek(dicionario, 28, 1);
 	}

 	fclose(dicionario);

	return 1;
}

void remover_tabela(){
	char *nomeTabela=(char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
    //~ struct fs_objects objeto;
	//~ int erro;
	
	printf("Digite o nome da tabela que deseja remover!\n");
	scanf("%s",nomeTabela);

	printf("\nRemovendo tabela %s...\n", nomeTabela);
	
	//Código para remoção
	excluir_arquivo(nomeTabela);
}


int menu(){
	int opcao;
	//~ int tipo_tabela;
	
	//Menu
	printf("\nO que deseja fazer?\n");
	printf("1 - Consultar tabelas\n2 - Criar tabelas\n3 - Remover Tabela\n0 - Sair\n");
	scanf("%d",&opcao);

	switch(opcao){
		case 1:
			consultar();
		break;

		case 2:
			//Descomentar quando criação dinamica de tabelas for implementado
			//~ printf("1-Criar uma tabela especifica\n2-Criar tabelas pre-estabelecidas\n");
			//~ scanf("%d",&tipo_tabela);
			//~ tipo_tabela == 1 ? criar_tabela() : criar_tabelas_padrao();
			criar_tabelas_padrao();
			printf("\n");
		break;

		case 3:
			remover_tabela();
		break;
		
		case 0:
			return 0;
		break;
	}
	menu();

	return 0;
}

//Criar tabela para pk e fks
int criar_tabela_chaves(){
	int erro;
	table *t = NULL;

	t = iniciaTabela("BD_Chaves");

	if(t == ERRO_NOME_TABELA_INVALIDO){
		printf("Erro: na função iniciaTabela(). Nome da tabela já existente.\n");
		return 0;
	}

	t = adicionaCampo(t, "TabelaOrigem", 'S', 20);
	t = adicionaCampo(t, "ID", 'I', (sizeof(int)));
	t = adicionaCampo(t, "Tipo", 'C', (sizeof(char)));
	t = adicionaCampo(t, "TabelaDestino", 'S', 20);
	t = adicionaCampo(t, "CampoOrigem", 'S', 10);
	t = adicionaCampo(t, "CampoDestino", 'S', 10);
	erro = finalizaTabela(t);

	if(erro != SUCCESS){
		printf("Erro %d: na função finalizaTabela().\n", erro);
		return 0;
	}
	
	return 1;
}

void inicializacao(){
	criar_tabela_chaves();
}

int main(){

	//~ inicializacao();

	menu();

	return 0;
}
