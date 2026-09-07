# 32 - vocabulario das portas: o acucar que se esconde e a fase com dois nomes

- **Status:** ABERTA — precisa de decisao do dono (as duas mexem em API publica)
- **Categoria:** Coerencia de API
- **Registrada em:** 2026-09-07, na revisao arquitetural (achados 2.5, 2.11 e a metade de engine do 1.4)

Tres incoerencias pequenas de vocabulario. Nenhuma e defeito de logica; as tres
cobram do consumidor, sempre da mesma forma: **ele tropeca uma vez, resolve, e
nunca escreve por que.**

---

## A. O acucar de enum some quando alguem herda (achado 2.5)

`audio::Player` declara duas coisas com o mesmo nome:

```cpp
virtual void play(SoundId id) = 0;              // o despacho

template <typename Sound> requires std::is_enum_v<Sound>
void play(const Sound sound) { play(static_cast<SoundId>(sound)); }   // o acucar
```

Qualquer derivada que faca `override` no primeiro **esconde** o segundo. Chamar
`p.play(Sound::Jump)` atraves do tipo concreto para de compilar; atraves de
`Player&` funciona.

**A prova de que isto morde ja esta no ecossistema:** `ForgeAudio.h:71` tem a
linha `using cengine::audio::Player::play;`. O remendo existe, funciona, e e
obrigacao de quem herda — esquece-lo so da erro no call site do jogo, longe da
causa.

### Saidas

1. **Funcao livre:** `template<class S> void play(Player& p, S s)`. O acucar sai
   da classe e o problema deixa de existir. Muda o call site
   (`play(player, Sound::Jump)`), que e menos bonito.
2. **Renomear o virtual:** `virtual void playId(SoundId)` + um `play` nao-virtual
   que despacha. O call site nao muda; muda a assinatura que TODO backend
   implementa (hoje: um).
3. **Deixar como esta e documentar o `using` como parte do contrato.** Custa
   nada e mantem o tropeco.

> A (2) parece a melhor: hoje ha **um** implementador (`forgeaudio::AudioPlayer`,
> neste workspace), entao o custo real e uma linha.

---

## B. A mesma fase do laco tem dois nomes (achado 2.11)

`IScene::draw()` e `IGameManager::render()` sao a MESMA fase.
`GameManager::render()` e literalmente `scene.draw();`.

Num projeto que trata vocabulario como parte da entrega, e um desencontro que
todo consumidor encontra uma vez.

### Saidas

1. **Um nome so, com depreciacao.** Escolher `draw` (o que as cenas falam, e o
   que os 14 jogos escrevem) ou `render`.
2. **Deixar, e escrever por que sao dois.** Se a distincao for real — o gerente
   ORQUESTRA o quadro, a cena DESENHA — ela merece uma frase, nao silencio.

> **Custo real:** renomear `IGameManager::render` alcanca todo implementador de
> `IGameManager`. Neste workspace: `routing::GameManager`,
> `diorama::app::GerenteDeCenaUnica`, e os adaptadores do 8puzzle e do
> spaceinvaders (fase 2, modo hospedado). Os dois ultimos sao REFERENCIA.

---

## C. Existem dois `Mat4` e nenhuma ponte oficial (achado 1.4)

`cengine::camera3d::Mat4` (16 floats, coluna-maior) e o `mat4` do
ModifiedSonyMath, que e o que `forgemesh::setCamera` recebe. A traducao mora num
namespace anonimo dentro do `ForgeMalhaScene.cpp` do `diorama` — ou seja, **o
proximo consumidor vai reescreve-la.**

Num ecossistema cujo risco numero um documentado e *"trocar uma convencao produz
uma imagem plausivel e errada"*, essa e a peca que menos pode ser copiada.

### Saidas

1. **O casco aceita os 16 floats:** um overload
   `forgemesh::setCamera(const float (&colunaMaior)[16])`. A cengine nao aprende
   nada de The-Forge e o casco nao aprende nada da cengine — os dois falam
   `float[16]`, que e o formato do constant buffer de qualquer jeito.
2. **A cengine expoe `data()`** e o consumidor faz o `memcpy`. Mesma coisa, com o
   erro possivel devolvido para quem chama.
3. **Deixar no lab** ate haver um segundo consumidor (o criterio do ADR 0002).

> A (1) e a unica que poe a conversao num lugar TESTAVEL, e a (3) e a que o ADR
> 0002 pediria. A tensao e real: e mecanismo de UMA linha com UM consumidor —
> mas a linha e da familia que produz imagem espelhada.

---

## A pergunta que fica para o dono

> As tres sao independentes. A (A) custa uma linha e tem um implementador; a (B)
> alcanca projetos de referencia; a (C) e uma aposta contra o ADR 0002.
>
> Alguma delas entra agora?
