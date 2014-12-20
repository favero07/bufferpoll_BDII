#include "buffend.h"
// LEITURA DE DICIONARIO E ESQUEMA
struct fs_objects leObjeto(char *nTabela){

	FILE *dicionario;
	char *tupla = (char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
	int cod;
	dicionario = fopen("fs_object.dat", "a+b"); // Abre o dicionario de dados.

	struct fs_objects objeto;

	if(!verificaNomeTabela(nTabela)){
		printf("Erro GRAVE! na função leObjeto(). Nome da tabela inválido.\nAbortando...\n");
		exit(1);
	}

	if (dicionario == NULL)	{
		printf("Erro GRAVE! na função leObjeto(). Arquivo não encontrado.\nAbortando...\n\n");
		exit(1);
	}
	

	while(fgetc (dicionario) != EOF){
        fseek(dicionario, -1, 1);

        fread(tupla, sizeof(char), TAMANHO_NOME_TABELA , dicionario); //Lê somente o nome da tabela

        if(strcmp(tupla, nTabela) == 0){ // Verifica se o nome dado pelo usuario existe no dicionario de dados.
      		strcpy(objeto.nome, tupla);
      		fread(&cod,sizeof(int),1,dicionario);	// Copia valores referentes a tabela pesquisada para a estrutura.
      		objeto.cod=cod;
      		fread(tupla,sizeof(char),TAMANHO_NOME_TABELA,dicionario);
      		strcpy(objeto.nArquivo, tupla);
      		fread(&cod,sizeof(int),1,dicionario);
      		objeto.qtdCampos = cod;
      		
        	return objeto;
        }
        fseek(dicionario, 28, 1); // Pula a quantidade de caracteres para a proxima verificacao(4B do codigo, 20B do nome do arquivo e 4B da quantidade de campos).
	}
	return objeto;
}
tp_table *leSchema (struct fs_objects objeto){
	FILE *schema;
	int i = 0, cod;
	char *tupla = (char *)malloc(sizeof(char)*TAMANHO_NOME_CAMPO);  
	tp_table *esquema = (tp_table *)malloc(sizeof(tp_table)*objeto.qtdCampos); // Aloca esquema com a quantidade de campos necessarios.

	if(esquema == NULL)
		return ERRO_DE_ALOCACAO;

	schema = fopen("fs_schema.dat", "a+b"); // Abre o arquivo de esquemas de tabelas.

	if (schema == NULL)
		return ERRO_ABRIR_ESQUEMA;

	while((fgetc (schema) != EOF) && (i < objeto.qtdCampos)){ // Varre o arquivo ate encontrar todos os campos com o codigo da tabela.
        fseek(schema, -1, 1);

        if(fread(&cod, sizeof(int), 1, schema)){ // Le o codigo da tabela.
        	if(cod == objeto.cod){ // Verifica se o campo a ser copiado e da tabela que esta na estrutura fs_objects.
        		
        		fread(tupla, sizeof(char), TAMANHO_NOME_CAMPO, schema);
        		strcpy(esquema[i].nome,tupla);					// Copia dados do campo para o esquema.
        		fread(&esquema[i].tipo, sizeof(char),1,schema);      
        		fread(&esquema[i].tam, sizeof(int),1,schema);   
        		i++;    		
        	}
        	else
        		fseek(schema, 45, 1); // Pula a quantidade de caracteres para a proxima verificacao (40B do nome, 1B do tipo e 4B do tamanho).
        }
        
    }
    return esquema;
}
//--------------------------------------------------
// INICIALIZACAO DO BUFFER
tp_buffer * initbuffer(){
	
	tp_buffer *bp = (tp_buffer*)malloc(sizeof(tp_buffer)*PAGES);
	int i;
	if(bp == NULL)
		return ERRO_DE_ALOCACAO;
	for (i = 0;i < PAGES; i++){
		bp->db=0;
		bp->pc=0;
		bp->nrec=0;
		bp++;
	}
	return bp;
}
//--------------------------------------------------
// IMPRESSAO BUFFER
void cria_campo(int tam, int header, char *val, int x){
	int i;
	char aux[30];

	if(header){
		for(i = 0; i <= 30 && val[i] != '\0'; i++){
			aux[i] = val[i];
		}
		for(;i<30;i++)
			aux[i] = ' ';
		aux[i] ='\0';
		printf("%s", aux);

	}

	else{
		for(i = 0; i < x; i++)
			printf(" ");
	}
}
int drawline(tp_buffer *buffpoll, tp_table *s, struct fs_objects objeto, int p, int num_page){
	 
	if (num_page > PAGES || p > SIZE){
		return ERRO_DE_PARAMETRO;
	}
	int *pos_ini, aux = (p * tamTupla(s,objeto)) , num_reg = objeto.qtdCampos;
	pos_ini = &aux;
	int count, pos_aux, bit_pos;
	union c_double cd;
	union c_int ci;
	int x = 0;
	
	count = pos_aux = bit_pos = 0;
	
	for(count = 0; count < num_reg; count++){
		pos_aux = *(pos_ini);
		bit_pos = 0;
		

		switch(s[count].tipo){
			
			case 'S':
				x = 0;
				while(buffpoll[num_page].data[pos_aux] != '\0'){
			
					printf("%c", buffpoll[num_page].data[pos_aux]);
					if ((buffpoll[num_page].data[pos_aux++] & 0xc0) != 0x80) bit_pos++; //Conta apenas bits que possam ser impressos (UTF8)
				x++;
				}
				
				cria_campo((TAMANHO_NOME_CAMPO - (bit_pos)), 0, (char*)' ', (30 - x));
				break;
			
			case 'I':
				while(pos_aux < *(pos_ini) + s[count].tam){
					ci.cnum[bit_pos++] = buffpoll[num_page].data[pos_aux++];
				}
				printf("%d", ci.num); //Controla o número de casas até a centena
				cria_campo((TAMANHO_NOME_CAMPO - (bit_pos)), 0, (char*)' ', 28);
				break;
				
			case 'D':
				while(pos_aux < *(pos_ini) + s[count].tam){
					cd.double_cnum[bit_pos++] = buffpoll[num_page].data[pos_aux++]; // Cópias os bytes do double para área de memória da union
				}
				printf("%.3lf", cd.dnum);
				cria_campo((TAMANHO_NOME_CAMPO - (bit_pos)), 0, (char*)' ', 24);
				break;
			
			case 'C': 
				printf("%c", buffpoll[num_page].data[pos_aux]);
				if(s[count].tam < strlen(s[count].nome)){
					bit_pos = strlen(s[count].nome);
				}
				else{
					bit_pos = s[count].tam;
				}
				cria_campo((bit_pos - 1), 0, (char*)' ', 29);	
				break;
			
			default: 
				return ERRO_IMPRESSAO;
				break;
		}
		*(pos_ini) += s[count].tam;		
	}
	printf("\n");
	return SUCCESS;
}
int cabecalho(tp_table *s, int num_reg){
	int count, aux;
	aux = 0;
	
	for(count = 0; count < num_reg; count++){
		cria_campo(s[count].tam, 1, s[count].nome, 0); // O segundo parâmetro significa se = 1 Cabecalho se = 0 é valor daquele campo
		aux += s[count].tam;
	}
	printf("\n");
	return aux;
}
int printbufferpoll(tp_buffer *buffpoll, tp_table *s,struct fs_objects objeto, int num_page){
	
	int aux, i, num_reg = objeto.qtdCampos;
	
	
	if(buffpoll[num_page].nrec == 0){
		return ERRO_IMPRESSAO;	
	}
	
	i = aux = 0;
	
	aux = cabecalho(s, num_reg);
	

	while(i < buffpoll[num_page].nrec){ // Enquanto i < numero de registros * tamanho de uma instancia da tabela
		drawline(buffpoll, s, objeto, i, num_page);
		i++;
	}
	return SUCCESS;
}
//--------------------------------------------------
// FUNCOES AUXILIARES
int tamTupla(tp_table *esquema, struct fs_objects objeto){// Retorna o tamanho total da tupla da tabela.

    int qtdCampos = objeto.qtdCampos, i, tamanhoGeral = 0;
   
    if(qtdCampos){ // Lê o primeiro inteiro que representa a quantidade de campos da tabela.
		for(i = 0; i < qtdCampos; i++)
			tamanhoGeral += esquema[i].tam; // Soma os tamanhos de cada campo da tabela.
	}

	return tamanhoGeral;
}
int pot10(int n)
{
	if(n == 0)
		return 1;
	return 10 * pot10(n-1);
}
int strtam(char n[])
{
	if(n[0]==0)
		return 0;
	return 1+strtam(n+1);
}
int convertI(char u[])
{
	if(strtam(u) == 0)
		return 0;
	return (u[0]-48)*pot10(strtam(u)-1) + convertI(u+1);
}
double get_decimal(char u[]) 
{
	double decimal=0;
	int i,tamanho = strtam(u);
	for(i=0;i<tamanho && u[i]!='.';i++); // posiciona o indice i encima do ponto
	decimal=convertI(u+i+1);///(pot10(tamanho-i-1));
	decimal=(double)convertI(u+i+1)/(double)(pot10(tamanho-i-1));
	return decimal;
}
double get_inteiro(char v[]) 
{
	double inteiro=0;
	int i,tamanho = strtam(v);
	char u[10];
	strcpy(u,v); //copia o vetor para uma vaiável auxiliar para preservar sua originalidade
	for(i=0;i<tamanho && u[i]!='.';i++); // posiciona o indice i encima do ponto
	u[i]=0; // separa a parte inteira da parte decimal, inserindo um null no lugar do ponto
	inteiro=convertI(u);
	return inteiro;
}
double convertD(char u[]) 
{
	return get_inteiro(u)+get_decimal(u);
	//Soma inteiro com decimal.ss
}
int verificaNomeTabela(char *nomeTabela)
{

	FILE *dicionario;
	char *tupla = (char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
	if((dicionario = fopen("fs_object.dat","a+b")) == NULL)
    	return ERRO_ABRIR_ARQUIVO;

    while(fgetc (dicionario) != EOF){
        fseek(dicionario, -1, 1);

        fread(tupla, sizeof(char), TAMANHO_NOME_TABELA, dicionario); //Lê somente o nome da tabela

        if(strcmp(tupla, nomeTabela) == 0){ // Verifica se o nome dado pelo usuario existe no dicionario de dados.
        	return 1;
        }
        
        fseek(dicionario, 28, 1);
 	}

 	fclose(dicionario);
 	return 0;
}
int quantidadeTabelas()
{

	FILE *dicionario;
	int codTbl = 0;

    if((dicionario = fopen("fs_object.dat","a+b")) == NULL)
    	return ERRO_ABRIR_ARQUIVO;

	while(fgetc (dicionario) != EOF){
        fseek(dicionario, -1, 1);

       	codTbl++; // Conta quantas vezes é lido uma tupla no dicionario.

        fseek(dicionario, 48, 1);
 	}

 	fclose(dicionario);

 	return codTbl;
}
//---------------------------------------
// INSERE UMA TUPLA NO BUFFER!
char *getTupla(tp_table *campos,struct fs_objects objeto, int from){ //Pega uma tupla do disco a partir do valor de from

    int tamTpl = tamTupla(campos, objeto);
    char *linha=(char *)malloc(sizeof(char)*tamTpl);
    FILE *dados;

    from = from * tamTpl;

    dados = fopen(objeto.nArquivo, "r");

	if (dados == NULL)
		return ERRO_DE_LEITURA;

    fseek(dados, from, 1);
    if(fgetc (dados) != EOF){
        fseek(dados, -1, 1);
        fread(linha, sizeof(char), tamTpl, dados); //Traz a tupla inteira do arquivo
    }
    else{       //Caso em que o from possui uma valor inválido para o arquivo de dados
        fclose(dados);
        return ERRO_DE_LEITURA;
    }
    fclose(dados);
    return linha;
}
void setTupla(tp_buffer *buffer,char *tupla, int tam, int pos){ //Coloca uma tupla de tamanho "tam" no buffer e na página "pos"
	int i=buffer[pos].position;
	for (;i<buffer[pos].position + tam;i++)
        buffer[pos].data[i] = *(tupla++);
}
int colocaTuplaBuffer(tp_buffer *buffer, int from, tp_table *campos, struct fs_objects objeto){//Define a página que será incluida uma nova tupla
	
	char *tupla = getTupla(campos,objeto,from);

 	if(tupla == ERRO_DE_LEITURA)
 		return ERRO_LEITURA_DADOS;

    int i=0, found=0;
	while (!found && i < PAGES)//Procura pagina com espaço para a tupla.
	{
    	if(SIZE - buffer[i].position > tamTupla(campos, objeto)){// Se na pagina i do buffer tiver espaço para a tupla, coloca tupla.
            setTupla(buffer, tupla, tamTupla(campos, objeto), i);
            found = 1;
            buffer[i].position += tamTupla(campos, objeto); // Atualiza proxima posição vaga dentro da pagina.
            buffer[i].nrec += 1;
            break;
    	}
    	i++;// Se não, passa pra proxima página do buffer.
    }
    
    if (!found)
		return ERRO_BUFFER_CHEIO;

	return SUCCESS;
}
//----------------------------------------
// CRIA TABELA
table *iniciaTabela(char *nome)
{
	if(verificaNomeTabela(nome)){	// Se o nome já existir no dicionario, retorna erro.
		return ERRO_NOME_TABELA_INVALIDO;
	}

	table *t = (table *)malloc(sizeof(table)*1);
	strcpy(t->nome,nome); // Inicia a estrutura de tabela com o nome da tabela.
	t->esquema = NULL; // Inicia o esquema da tabela com NULL.
	return t; // Retorna estrutura para criação de uma tabela.
}
table *adicionaCampo(table *t,char *nomeCampo, char tipoCampo, int tamanhoCampo)
{
	if(t == NULL) // Se a estrutura passada for nula, retorna erro.
		return ERRO_ESTRUTURA_TABELA_NULA;
	tp_table *aux;  
	if(t->esquema == NULL) // Se o campo for o primeiro a ser adicionado, adiciona campo no esquema.
	{
		tp_table *e = (tp_table *)malloc(sizeof(tp_table)*1);
		e->next = NULL;
		strcpy(e->nome, nomeCampo); // Copia nome do campo passado para o esquema
		e->tipo = tipoCampo; // Copia tipo do campo passado para o esquema
		e->tam = tamanhoCampo; // Copia tamanho do campo passado para o esquema
		t->esquema = e; 
		return t; // Retorna a estrutura
	}
	else
	{ 
		for(aux = t->esquema; aux != NULL; aux = aux->next) // Anda até o final da estrutura de campos.
		{
			if(aux->next == NULL) // Adiciona um campo no final.
			{
				tp_table *e = (tp_table *)malloc(sizeof(tp_table)*1);
				e->next = NULL;
				strcpy(e->nome, nomeCampo);
				e->tipo = tipoCampo;
				e->tam = tamanhoCampo;
				aux->next = e; // Faz o campo anterior apontar para o campo inserido.
				return t;
			}
		}
	}

	return t; //Retorna estrutura atualizada.
}
int finalizaTabela(table *t)
{
	if(t == NULL)
		return ERRO_DE_PARAMETRO;

	FILE *esquema, *dicionario;
	tp_table *aux;
	int codTbl = quantidadeTabelas() + 1, qtdCampos = 0; // Conta a quantidade de tabelas já no dicionario e soma 1 no codigo dessa nova tabela.
	char nomeArquivo[TAMANHO_NOME_ARQUIVO];

	if((esquema = fopen("fs_schema.dat","a+b")) == NULL)
        return ERRO_ABRIR_ARQUIVO;

	for(aux = t->esquema; aux!=NULL; aux = aux->next) // Salva novos campos no esquema da tabela, fs_schema.dat
	{
		fwrite(&codTbl,sizeof(codTbl),1,esquema);
		fwrite(&aux->nome,sizeof(aux->nome),1,esquema);
		fwrite(&aux->tipo,sizeof(aux->tipo),1,esquema);
		fwrite(&aux->tam,sizeof(aux->tam),1,esquema);

		qtdCampos++; // Soma quantidade total de campos inseridos.
	}

	fclose(esquema);

	if((dicionario = fopen("fs_object.dat","a+b")) == NULL)
    	return ERRO_ABRIR_ARQUIVO;

	strcpy(nomeArquivo, t->nome);
	strcat(nomeArquivo, ".dat\0");
	strcat(t->nome, "\0");
	// Salva dados sobre a tabela no dicionario.
    fwrite(&t->nome,sizeof(t->nome),1,dicionario);
	fwrite(&codTbl,sizeof(codTbl),1,dicionario);
	fwrite(&nomeArquivo,sizeof(nomeArquivo),1,dicionario);
	fwrite(&qtdCampos,sizeof(qtdCampos),1,dicionario);

	fclose(dicionario);
	return SUCCESS;
}
//-----------------------------------------
// INSERE NA TABELA
column *insereValor(column *c, char *nomeCampo, char *valorCampo)
{
	
	column *aux;
	if(c == NULL) // Se o valor a ser inserido é o primeiro, adiciona primeiro campo.
	{
		column *e = (column *)malloc(sizeof(column)*1);
		e->valorCampo = (char *)malloc(sizeof(char) * (sizeof(valorCampo)));
		strcpy(e->nomeCampo, nomeCampo); 
		strcpy(e->valorCampo, valorCampo);
		e->next = NULL;
		c = e;
		return c;
	} 
	else
	{
		for(aux = c; aux != NULL; aux = aux->next) // Anda até o final da lista de valores a serem inseridos e adiciona um novo valor.
		{
			if(aux->next == NULL)
			{
				column *e = (column *)malloc(sizeof(column)*1);
				e->valorCampo = (char *)malloc(sizeof(char) * (sizeof(valorCampo)));
				e->next = NULL;
				strcpy(e->nomeCampo, nomeCampo);
				strcpy(e->valorCampo, valorCampo);
				aux->next = e;
				return c;
			}
		}
	}

	return ERRO_INSERIR_VALOR;
}

// Verifica se o arquivo existe
// Retorna 1 se existir e 0 se não existir
int existe_arquivo(const char* nomeArquivo){
	FILE* arquivo = fopen(nomeArquivo, "r"); 
	if(arquivo!=NULL){
		fclose(arquivo);
		
		return 1;
	}
	return 0;
}

//Procura se valor da busca existe na tabela
//Retorna 1 se encontrar e 0 se não encontrar
int TabelaPossuiPk(char tabela[]){
	int erro;
	
	struct fs_objects objeto = leObjeto("BD_Chaves");

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

	// Auxiliar dos dados da tupla buscada
	int j, y = 0;

	for(j=0; j < objeto.qtdCampos*bufferpoll[0].nrec; j++){
		if(strcmp(pagina[j].valorCampo,tabela) == 0){
			for(y=j;y<j+objeto.qtdCampos;y++){
				if (strcmp (pagina[y].nomeCampo, "Tipo") == 0){
					if (strcmp (pagina[y].valorCampo, "P") == 0)
						return 1; // Encontrou a tupla, retorna 1
				}
			}
		}
	}

	return 0; // Nao encontrou a tupla, retorna 0
}

int campo_existe_na_tabela(char tabela[],char campo[]){
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

	// Auxiliar dos dados da tupla buscada
	int j, y = 0;

	for(j=0; j < objeto.qtdCampos*bufferpoll[0].nrec; j++){
		if(strcmp(pagina[j].nomeCampo,tabela) == 0){
			for(y=j;y<j+objeto.qtdCampos;y++){
				if (strcmp (pagina[y].nomeCampo, campo) == 0)
					return 1; // Encontrou a coluna(campo), retorna 1
			}
		}
	}

	return 0; // Nao encontrou a coluna, retorna 0
}

int finalizaInsert(char *nome, column *c)
{
	column *auxC;
	int i = 0, x = 0, t;
	FILE *dados;

	if(!verificaNomeTabela(nome)){
		printf("\nA Tabela informada nao existe!\n");
		return 0;
	}

	struct fs_objects dicio = leObjeto(nome); // Le dicionario
	tp_table *auxT = leSchema(dicio); // Le esquema

	//se estiver inserindo na tabela de chaves, verifica regras de PK e FK
	if (strcmp (nome, "BD_Chaves") == 0){
		column *auxP;
		char *tabela_fk=(char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
		char *campo_fk=(char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);
		int p, fk;
		
		if(existe_arquivo("BD_Chaves.dat")){
			int pk;
			for(auxP = c, p = 0; auxP != NULL; auxP = auxP->next, p++)
			{
				if(p >= dicio.qtdCampos)
					p = 0;

				if (pk != 1 && auxT[p].tipo == 'C'){
					//Se está tentando inserir PK, seta pk = 1 para verificar posteriormente se já existe PK 
					if (strcmp (auxP->valorCampo, "P") == 0){
						pk = 1;
						p=0;
						auxP = c;
					}
				}

				//Caso esteja inserindo PK, verifica se tabela já possui PK
				if (pk == 1 && strcmp (auxP->nomeCampo, "TabelaOrigem") == 0){
					printf("\nInserindo chave na tabela %s",auxP->valorCampo);
					if(TabelaPossuiPk(auxP->valorCampo))
						return ERRO_VIOLACAO_PK;
				}
				
				if (fk != 1 && auxT[p].tipo == 'C'){
					//Se está tentando inserir FK, seta Fk = 1 para verificar posteriormente se tabela da FK existe 
					if (strcmp (auxP->valorCampo, "F") == 0){
						fk = 1;
						p=0;
						auxP = c;
					}
				}

				//Caso esteja inserindo FK, verifica se tabela da FK existe
				if (fk == 1 && strcmp (auxP->nomeCampo, "TabelaDestino") == 0 && tabela_fk == NULL)
					strcpy(tabela_fk, auxP->valorCampo);

				if (fk == 1 && strcmp (auxP->nomeCampo, "CampoDestino") == 0 && campo_fk == NULL)
					strcpy(campo_fk, auxP->valorCampo);
			}
		}else{
			for(auxP = c, p = 0; auxP != NULL; auxP = auxP->next, p++)
			{
				if(p >= dicio.qtdCampos)
					p = 0;

				if (fk != 1 && auxT[p].tipo == 'C'){
					//Se está tentando inserir FK, seta Fk = 1 para verificar posteriormente se tabela da FK existe 
					if (strcmp (auxP->valorCampo, "F") == 0){
						fk = 1;
						p=0;
						auxP = c;
					}
				}

				//Caso esteja inserindo FK, verifica se tabela da FK existe
				if (fk == 1 && strcmp (auxP->nomeCampo, "TabelaDestino") == 0 && strcmp(tabela_fk, "\0") == 0)
					strcpy(tabela_fk, auxP->valorCampo);

				if (fk == 1 && strcmp (auxP->nomeCampo, "CampoDestino") == 0 && strcmp(campo_fk, "\0") == 0)
					strcpy(campo_fk, auxP->valorCampo);
			}
		}
		// Verifica se tabela destino da FK existe e se campo destino da Fk existe na tabela destino
		if(strcmp(tabela_fk, "\0") != 0 && strcmp(campo_fk, "\0") != 0){
			//Verifica se tabela destino da fk existe
			if(!verificaNomeTabela(tabela_fk)){
				printf("Erro: Tabela Destino da FK nao existe!\n");
				return 0;
			}

			// Verifica se campo da fk existe na tabela destino da fk
			if(campo_existe_na_tabela(tabela_fk,campo_fk) == 0){
				printf("Campo %s nao existe na tabela %s!\n",campo_fk, tabela_fk);
				return 0;
			}
		}
	}

	if((dados = fopen(dicio.nArquivo,"a+b")) == NULL)
    	return ERRO_ABRIR_ARQUIVO;

    char *tabela_origem=(char *)malloc(sizeof(char)*TAMANHO_NOME_TABELA);

	for(auxC = c, t = 0; auxC != NULL; auxC = auxC->next, t++)
	{
		if(t >= dicio.qtdCampos)
			t = 0;

		if(auxT[t].tipo == 'S'){ // Grava um dado do tipo string.
			if(sizeof(auxC->valorCampo) > auxT[t].tam){
				return ERRO_NO_TAMANHO_STRING;
			}
			if(strcmp(auxC->nomeCampo, auxT[t].nome) != 0){
				return ERRO_NOME_CAMPO;
			}
			char valorCampo[auxT[t].tam];
			strcpy(valorCampo, auxC->valorCampo);
			strcat(valorCampo, "\0");
			fwrite(&valorCampo,sizeof(valorCampo),1,dados);
		}
		else if(auxT[t].tipo == 'I'){ // Grava um dado do tipo inteiro.
			i = 0;
			while (i < strlen(auxC->valorCampo))
			{
				if(auxC->valorCampo[i] < 48 || auxC->valorCampo[i] > 57){ 
					return ERRO_NO_TIPO_INTEIRO;
				}
				i++;
			}

			int valorInteiro = convertI(auxC->valorCampo);
			fwrite(&valorInteiro,sizeof(valorInteiro),1,dados);
		}
		else if(auxT[t].tipo == 'D'){ // Grava um dado do tipo double.
			x = 0;
			while (x < strlen(auxC->valorCampo))
			{
				if((auxC->valorCampo[x] < 48 || auxC->valorCampo[x] > 57) && (auxC->valorCampo[x] != 46)){ 
					return ERRO_NO_TIPO_DOUBLE;
				}
				x++;
			}

			double valorDouble = convertD(auxC->valorCampo);
			fwrite(&valorDouble,sizeof(valorDouble),1,dados);
		}
		else if(auxT[t].tipo == 'C'){ // Grava um dado do tipo char.

			if(strlen(auxC->valorCampo) > (sizeof(char)))
			{
				return ERRO_NO_TIPO_CHAR;
			}
			char valorChar = auxC->valorCampo[0];
			fwrite(&valorChar,sizeof(valorChar),1,dados);
		}

		//Caso tenha criado chaves
		if (strcmp (auxC->nomeCampo, "TabelaOrigem") == 0)
			strcpy(tabela_origem, auxC->valorCampo);
		if (strcmp (auxC->nomeCampo, "TabelaDestino") == 0){
			if (strcmp (auxC->valorCampo, "\0") == 0)
				printf("\nChave primaria criada na tabela %s",tabela_origem);
			else
				printf("\nChave estrangeira criada na tabela %s",tabela_origem);
		}

	}
	fclose(dados);
	free(c); // Libera a memoria da estrutura.
	free(auxT); // Libera a memoria da estrutura.
	return SUCCESS;
}
//----------------------------------------
// EXCLUIR TUPLA BUFFER
column * excluirTuplaBuffer(tp_buffer *buffer, tp_table *campos, struct fs_objects objeto, int page, int nTupla){
	column *colunas = (column *)malloc(sizeof(column)*objeto.qtdCampos);

	if(colunas == NULL)
		return ERRO_DE_ALOCACAO;

	if(buffer[page].nrec == 0) //Essa página não possui registros
		return ERRO_PARAMETRO;
	
	int i, tamTpl = tamTupla(campos, objeto), j=0, t=0;
	i = tamTpl*nTupla; //Calcula onde começa o registro

	while(i < tamTpl*nTupla+tamTpl){
		t=0;
		
		colunas[j].valorCampo = (char *)malloc(sizeof(char)*campos[j].tam); //Aloca a quantidade necessária para cada campo
		colunas[j].tipoCampo = campos[j].tipo;	// Guarda o tipo do campo
		strcpy(colunas[j].nomeCampo, campos[j].nome); 	//Guarda o nome do campo
	
		while(t < campos[j].tam){
			colunas[j].valorCampo[t] = buffer[page].data[i];	//Copia os dados
			t++;
			i++;
		}
		j++;
	}
	j = i;
	i = tamTpl*nTupla;
	for(; i < buffer[page].position; i++, j++) //Desloca os bytes do buffer sobre a tupla excluida
		buffer[page].data[i] = buffer[page].data[j];

	buffer[page].position -= tamTpl;
	buffer[page].nrec--;

	return colunas; //Retorna a tupla excluida do buffer
}
//----------------------------------------
// RETORNA PAGINA DO BUFFER
column * getPage(tp_buffer *buffer, tp_table *campos, struct fs_objects objeto, int page){
	
	if(page > PAGES)
		return ERRO_PAGINA_INVALIDA;

	if(buffer[page].nrec == 0) //Essa página não possui registros
		return ERRO_PARAMETRO;
	
	column *colunas = (column *)malloc(sizeof(column)*objeto.qtdCampos*buffer[page].nrec); //Aloca a quantidade de campos necessária
	
	if(colunas == NULL)
		return ERRO_DE_ALOCACAO;
	
	int i=0, j=0, t=0, h=0;
	
	while(i < buffer[page].position){
		t=0;
		if(j >= objeto.qtdCampos)
			j=0;
		colunas[h].valorCampo = (char *)malloc(sizeof(char)*campos[j].tam);
		colunas[h].tipoCampo = campos[j].tipo;	//Guarda tipo do campo
		strcpy(colunas[h].nomeCampo, campos[j].nome); //Guarda nome do campo
		
		while(t < campos[j].tam){
			colunas[h].valorCampo[t] = buffer[page].data[i]; //Copia os dados
			t++;
			i++;
		}
		h++;
		j++;
	}
	return colunas; //Retorna a 'page' do buffer
}
//----------------------------------------
