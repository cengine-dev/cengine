# 22 - Colisao 2D: resolucao posicional (recorte a decidir)

- **Status:** estacionada para recorte - reavaliada em 2026-07-18 com o Zelda.
  Agora existem **2 de 2 evidencias** de resolucao posicional eixo-separada
  (mario-bros + zelda), mas **nenhum** dos dois consome penetracao/MTV. A
  evidencia nova dispara a comparacao do mecanismo; nao autoriza implementar a
  API original desta task. Ver "Reavaliacao com o Zelda".
- **Prioridade:** baixa/media - comparar as duas implementacoes eixo-separadas
  e promover apenas um nucleo puro que ambas realmente consumiriam. Se esse
  nucleo nao existir sem carregar politica de tiles/movimento, a candidata deve
  ser vetada, nao generalizada por antecipacao.
- **Categoria:** Arquitetura / extensao do modulo `collision2d`
- **Depende de:** 17 done (deteccao AABB + circulo).
- **Breaking:** esperado que nao. Qualquer recorte deve entrar como API NOVA e
  opt-in do `collision2d`, sem tocar o `intersects` existente; a assinatura so
  pode ser definida depois da comparacao Mario x Zelda.

## Avaliacao do gate (2026-07-15) - o mario resolveu, e nao precisou disto

O `collision2d` 0.7.0 responde so `bool intersects(...)` — nenhuma penetracao,
nenhum vetor de separacao. O mario-bros (degrau 2, dominio) foi o **primeiro**
consumidor do ecossistema a precisar de colisao com **RESOLUCAO** (quanto
empurrar o corpo para fora, por qual face) e nao so deteccao. Ele NAO destravou
a task, por dois motivos:

**1. Uma evidencia nao sao duas (ADR 0002, criterio 2).** Nenhum outro jogo VIVO
precisou de resolucao. O breakout resolve a *reflexao* dele (a bola quadrada) —
mas isso e outra coisa (inverter um vetor de velocidade), e mora no jogo
(`World::reflectOff`). A contagem de evidencias de RESOLUCAO POSICIONAL e UM: o
mario.

**2. Sinal mais forte: os dois jogos que "resolvem" resolvem DIFERENTE.** E o
ponto que mais empurra esta task para longe da engine:

  - o **breakout** resolve por **menor eixo de penetracao (MTV de forma-unica)**,
    porque a bola chega em diagonal e o que importa e por qual lado ela entrou;
  - o **mario** resolve **eixo-separado** (move X e resolve, move Y e resolve) —
    e nem precisou do MTV: a direcao do movimento do quadro ja diz por qual face
    empurrar. E a tecnica padrao de plataforma de tiles.

Se cada jogo resolve com uma estrategia diferente, "resolver" cheira a
**politica de jogo**, nao a mecanismo unico promovivel. Promover o MTV agora
daria a engine uma opiniao (eixo-separado? menor penetracao? swept?) que o
proximo consumidor pode nao compartilhar — exatamente o risco de deposito que a
ADR 0002 barra.

O que a engine PODERIA oferecer sem tomar essa opiniao e a **penetracao pura** (o
vetor de separacao minimo entre dois AABB), deixando a ESTRATEGIA de resolver no
jogo. Mas mesmo isso so se paga com um segundo consumidor querendo o mesmo
calculo — hoje o mario nem usa MTV.

**Novo gate (mais afiado):** comecar quando um **segundo** consumidor precisar de
penetracao/MTV **E** a forma de consumir esse calculo for a MESMA do primeiro
(mecanismo comum), e nao duas politicas divergentes. Enquanto cada jogo resolver
o contato do seu jeito, o calculo fica no jogo.

**Evidencia 1/2 registrada:** mario-bros, `mario::World::resolveHorizontal` /
`resolveVertical` (resolucao eixo-separado; nao usa MTV). O breakout NAO conta
como evidencia desta task — ele resolve reflexao, nao penetracao posicional, e
ja o faz no jogo (`reflectOff`).

## Reavaliacao ao fechar o mario (2026-07-16) - gate MANTIDO

O mario terminou completo (degraus 1-5: goombas, moedas, vidas, bandeira,
recordes por pontos/tempo) e o dono perguntou se valia uma task para "mover a
fisica para a cengine". Veredito: **nao** — a fisica de plataforma e feel
(constantes tunadas em playtest) + politica (estrategias de resolucao), e o
unico recorte de mecanismo dela e ESTA task; registro no bloco "Consideradas e
vetadas" do README do backlog.

O jogo completo ate ADICIONOU evidencia contra promover a resolucao: o **pisao
no goomba** (degrau 5a) e mais um contato "de cima" resolvido com regra propria
do jogo (matar + pulinho + so na metade superior do corpo), irmao da one-way e
diferente de ambos os anteriores. Tres formas de "resolver" em dois jogos, todas
politica. O gate segue: 2o consumidor consumindo penetracao/MTV do MESMO jeito.

## Reavaliacao com o Zelda (2026-07-18) - 2/2 do eixo-separado, 0/2 do MTV

O Zelda, task 02, trouxe a **2a evidencia real de resolucao posicional
eixo-separada**. Seu `World::moveAndResolveX/Y` repete a sequencia estrutural do
Mario: mover em X e empurrar pela face de entrada; depois mover em Y e empurrar
pela face de entrada. Nos dois jogos, ficar bloqueado num eixo preserva o
movimento no outro. A diferenca de genero confirma que o padrao nao e exclusivo
de plataforma: no Zelda os dois eixos sao dirigidos e simetricos, sem gravidade
nem tile one-way.

**Evidencia 2/2 registrada:** zelda, task 02 (done e validada jogando),
`zelda::World::moveAndResolveX` / `moveAndResolveY`, com testes de parar e
deslizar nas paredes em ambos os eixos.

A evidencia, porem, **nao e de penetracao/MTV**. Assim como o Mario, o Zelda ja
sabe o eixo e o sentido do movimento; nenhum dos dois calcula ou deseja um
vetor de separacao minimo generico. Implementar agora a `penetration()` proposta
abaixo continuaria sendo especulacao: teria 0 consumidores reais.

O gate do ADR 0002 foi atingido para **avaliar a extracao do mecanismo
eixo-separado**, nao para promover automaticamente uma estrategia de resolucao.
Antes de codigo na engine, e preciso colocar as duas implementacoes lado a lado
e identificar se sobra um nucleo puro, sem grade de tiles, `grounded`, eventos,
gravidade ou regras de dano, que os dois jogos realmente substituiriam. Se nao
sobrar uma API pequena e util, a repeticao fica nos jogos.

## Contexto

`cengine::collision2d` e um modulo de **deteccao**: `intersects(forma, forma) ->
bool` (task 17). Um plataforma precisa de mais para os corpos NAO se
atravessarem: dado que dois AABB se sobrepoem, *quanto* e *em que direcao*
separar. Isso e penetracao (um vetor), nao um booleano.

## Objetivo original (nao autorizado pela evidencia atual)

A proposta inicial era oferecer o **calculo de penetracao/MTV puro** — um vetor
de separacao minimo entre duas formas — sem resolver nada:

```cpp
namespace cengine::collision2d {
// Vetor de separacao minimo: quanto mover `a` para deixar de tocar `b`.
// nullopt quando nao se tocam. NAO move, NAO decide eixo, NAO decide o que a
// colisao significa — isso continua no jogo.
[[nodiscard]] std::optional<Vec2> penetration(const Aabb& a, const Aabb& b);
}
```

A engine responde "quanto se penetram?"; o jogo continua dono de mover a
entidade, escolher a estrategia (eixo-separado vs MTV), zerar velocidade, marcar
`grounded`, tocar som.

Esta API permanece como registro de uma hipotese, nao como plano de
implementacao. Mario e Zelda fornecem evidencia para o eixo-separado e nao para
MTV; o recorte da task deve ser redesenhado a partir deles antes de subir.

## Fora do Escopo (o corte mecanismo x politica)

- **A ESTRATEGIA de resolucao.** Eixo-separado (mario) vs menor-penetracao
  (breakout) vs swept/CCD e escolha do jogo — depende do movimento e do gosto.
  Promover uma delas seria dar a engine uma opiniao sobre COMO resolver.
- Mover entidades, zerar velocidade, `grounded`, plataforma one-way, pisar no
  inimigo — tudo politica, fica no jogo.
- Fisica, corpos rigidos, impulsos, atrito, sweep/CCD, 3D.
- Ownership de entidades.

## Criterios para decidir o recorte implementavel

Nao implementar imediatamente. A avaliacao pode comecar porque ha dois
consumidores, mas codigo so sobe quando:

- as implementacoes de Mario e Zelda forem comparadas linha a linha;
- um nucleo puro e testavel for identificado, sem politica de jogo;
- os dois jogos conseguirem consumir a API proposta de forma natural; e
- a API refletir o mecanismo real observado. `penetration()`/MTV so entra se
  surgir consumidor real para esse calculo.

## Criterios de Aceite (quando/se subir)

- [ ] Recorte puro no `collision2d`, opt-in, sem tocar `intersects` nem assumir
      uma politica de jogo.
- [ ] Testes cobrindo os casos reais de Mario e Zelda e as BORDAS (encostar).
- [ ] **Regra de proveniencia:** os testes de consumidor real citam a origem
      (repo @ commit, arquivo, linha) e transcrevem os valores do jogo.
- [ ] A engine NAO resolve: nenhuma entidade movida, nenhuma estrategia de eixo
      embutida.
- [ ] README separa o mecanismo geometrico promovido da estrategia e das
      consequencias de jogo, que continuam nos consumidores.

## Riscos

- Promover a ESTRATEGIA de um jogo (eixo-separado ou MTV) como se fosse
  universal, quebrando o proximo consumidor que resolve diferente.
- Deslizar de "medir penetracao" para "resolver colisao" e virar meio motor de
  fisica — fora do escopo do modulo de deteccao (ADR 0001/0002).

## Relacionado

- Task 17 - deteccao AABB/circulo (a base deste modulo).
- ADR 0002 - criterio de promocao (2 evidencias, mecanismo x politica).
- mario-bros, degrau 2 (`.ai/task/02-dominio-andar-pular-cair.md`) - o consumidor
  que resolve eixo-separado; evidencia 1/2.
- zelda, task 02 (`.ai/task/02-dominio-2-eixos.md`) - o segundo consumidor do
  eixo-separado; evidencia 2/2.
- breakout, `World::reflectOff` - resolve reflexao (nao penetracao) no jogo;
  ilustra que "resolver contato" ja apareceu em duas formas DIFERENTES.
