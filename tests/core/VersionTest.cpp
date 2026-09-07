#include <gtest/gtest.h>

#include <fstream>
#include <string>

// A VERSAO da engine mora em DOIS lugares, e um deles ficou para tras.
//
// Ate aqui o `CMakeLists.txt` dizia `0.15.0` enquanto o `CHANGELOG.md` ja
// abria com `## [0.16.0]`. Ninguem percebeu porque nada le os dois: o CMake nao
// abre o changelog, e o changelog nao e compilado.
//
// **Duas copias que precisam concordar sao deriva esperando acontecer** — e a
// resposta desta casa para isso ja existe, no `RigDoBlenderTest` do `diorama`:
// *transcricao sem quem a confira e deriva esperando acontecer*. Este teste e
// aquele, aplicado a versao.
//
// Ele nao unifica as duas fontes (o changelog e escrito a mao, e tem de ser).
// Ele so garante que a primeira secao dele e a versao que o build carrega.

namespace {

/// A primeira versao anunciada no CHANGELOG: o `x.y.z` do primeiro `## [...]`.
/// Devolve vazio se o arquivo nao abrir ou nao tiver secao nenhuma — e os dois
/// casos FALHAM o teste, em vez de pula-lo. Teste que pula quando nao acha o
/// arquivo e teste que some no dia em que mais importa.
std::string primeiraVersaoDoChangelog(const std::string& caminho)
{
    std::ifstream arquivo(caminho);
    if (!arquivo)
    {
        return {};
    }

    std::string linha;
    while (std::getline(arquivo, linha))
    {
        // `## [0.17.0] - 2026-09-07`, e nao `## [Unreleased]`.
        if (linha.rfind("## [", 0) != 0)
        {
            continue;
        }
        const auto fim = linha.find(']', 4);
        if (fim == std::string::npos)
        {
            continue;
        }
        const std::string versao = linha.substr(4, fim - 4);
        if (versao.find_first_not_of("0123456789.") == std::string::npos && !versao.empty())
        {
            return versao;
        }
    }
    return {};
}

} // namespace

TEST(VersionTest, OCHANGELOGEOCMakeAnunciamAMesmaVersao)
{
    const std::string doChangelog = primeiraVersaoDoChangelog(std::string{ CENGINE_RAIZ } + "/CHANGELOG.md");

    ASSERT_FALSE(doChangelog.empty())
        << "CHANGELOG.md nao abriu ou nao tem uma secao '## [x.y.z]' — "
           "o teste FALHA em vez de pular, de proposito";

    EXPECT_EQ(doChangelog, std::string{ CENGINE_VERSION })
        << "a versao do CMakeLists.txt e a do topo do CHANGELOG.md divergiram";
}
