# 31 - collision2d: a borda responde de dois jeitos

- **Status:** ABERTA — precisa de decisao do dono (colide com o ADR 0003)
- **Categoria:** Coerencia de contrato num modulo ja promovido
- **Registrada em:** 2026-09-07, na revisao arquitetural (achado 2.4)

## O fato

`modules/collision2d/src/Intersects.cpp` responde a mesma pergunta com dois
criterios de borda:

```cpp
bool intersects(const Aabb& a, const Aabb& b)
{
    return a.x < b.x + b.w && ...;          // ENCOSTAR NAO CONTA
}

bool intersects(const Circle& a, const Circle& b)
{
    return dx * dx + dy * dy <= reach * reach;  // ENCOSTAR CONTA
}
```

Esta documentado nos dois lugares, e cada um tem uma razao boa por escrito:
retangulo pensa em AREA de sobreposicao (zero nao e sobreposicao), circulo pensa
em ALCANCE (tangenciar e o limiar natural, e evita o tiro que passa raspando).

**O problema nao e nenhuma das duas escolhas: e elas conviverem sem nome.** Um
jogo que troque a forma de um corpo — de caixa para circulo, ou o contrario —
muda o comportamento na borda sem mudar uma linha de logica.

## Onde isso ja vaza hoje

`camera2d::visible()` (`Viewport.cpp:18`) monta um `Aabb` da janela e chama o
`intersects` de retangulos. Com `cullMargin = 0`, um corpo **exatamente colado**
na borda da viewport e considerado invisivel e some.

A `cullMargin` nao existe para isso — ela existe para a borda nao PISCAR durante
a rolagem — mas e ela que hoje esconde o caso. Um consumidor que passe margem
zero encontra o defeito, e o sintoma e "o inimigo sumiu por um quadro".

## O que torna isto dificil, e nao e a conta

**Os consumidores estao congelados.** Este modulo foi promovido com evidencia do
spaceinvaders (AABB) e do asteroids (circulo), e os dois estao estacionados. Pelo
ADR 0002 (Emenda 1) eles continuam contando como evidencia; pelo ADR 0003 eles
nao voltam a ser compilados.

Trocar `<` por `<=` no AABB muda o comportamento de um caso de borda em jogos que
ninguem vai reabrir para conferir. **A suite desta engine encarna o caso de uso
deles** — entao o teste que quebrar (se algum quebrar) e o unico sinal que vai
existir.

## As tres saidas

### A. Unificar em `<=` (tocar conta)

- **A favor:** uma regra so, e ela e a que o consumidor de circulo ja tem. A
  troca de forma deixa de mudar o comportamento.
- **Contra:** muda o AABB, que e a metade com dois consumidores congelados. Um
  tiro que hoje passa raspando de um invasor passaria a acertar.
- **Custo:** a linha, mais os testes de borda da suite que assumem o `<`.

### B. Unificar em `<` (tocar nao conta)

- **A favor:** preserva o comportamento do AABB, que e o mais usado.
- **Contra:** o circulo perde o limiar natural. Um tiro exatamente tangente a
  rocha do asteroids deixa de acertar — e a documentacao atual diz, com todas as
  letras, que aquilo foi deliberado.
- **Veredito preliminar:** troca um comportamento desejado por coerencia. Ruim.

### C. Nomear a diferenca em vez de apaga-la

Duas funcoes, e o nome carrega o criterio:

```cpp
[[nodiscard]] bool overlaps(const Aabb&, const Aabb&);  // area > 0
[[nodiscard]] bool touches(const Aabb&, const Aabb&);   // area >= 0
```

- **A favor:** nao muda comportamento de ninguem; o `intersects` atual vira um
  alias do que ja fazia. Quem escolhe passa a escolher **por escrito**.
- **Contra:** a API cresce, e o ADR 0002 recusa deposito. Precisaria de evidencia
  de que alguem quer os dois — e hoje nao ha.
- **Nota:** e a unica das tres que resolve o vazamento do `camera2d` sem tocar em
  jogo nenhum: o `visible()` passaria a chamar `touches`, que e o que ele sempre
  quis dizer.

## A pergunta que fica para o dono

> A borda e uma decisao da ENGINE (uma regra so, opcao A) ou do JOGO (duas
> funcoes com nome, opcao C)?

O corte da casa — *"a engine responde se tocam; o que a colisao SIGNIFICA e do
jogo"* — puxa para (C). O gosto por API minima puxa para (A).

## Criterios de aceite (quando a decisao existir)

1. Um teste que fixe o comportamento da borda para as QUATRO combinacoes
   (retangulo/retangulo, circulo/circulo, circulo/retangulo, retangulo/circulo).
   Hoje a simetria e testada, o criterio de borda nao.
2. `camera2d::visible()` com `cullMargin = 0` nao descarta um retangulo colado na
   borda — ou o cabecalho passa a dizer que descarta, de proposito.
3. Se a escolha mudar o AABB: a suite desta engine acusa, e o CHANGELOG diz que e
   breaking para quem depende da borda.
