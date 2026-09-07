#include <cengine/camera3d/Camera.hpp>

#include <cmath>

namespace cengine::camera3d {
namespace {

Vec3 sub(const Vec3& a, const Vec3& b) { return { a.x - b.x, a.y - b.y, a.z - b.z }; }

Vec3 cross(const Vec3& a, const Vec3& b)
{
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
}

float dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 normalize(const Vec3& v)
{
    const float len = std::sqrt(dot(v, v));
    if (len <= 1e-8f)
    {
        return {};
    }
    return { v.x / len, v.y / len, v.z / len };
}

// Acesso coluna-maior: `at(m, coluna, linha)`.
constexpr float kPi = 3.14159265358979323846f;

float& at(Mat4& m, const int col, const int row) { return m.m[col * 4 + row]; }
float  at(const Mat4& m, const int col, const int row) { return m.m[col * 4 + row]; }

} // namespace

Mat4 Mat4::identity()
{
    Mat4 r;
    r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
    return r;
}

Vec3 eye(const Orbit& orbit)
{
    // Esfericas com **elevacao a partir do chao** (ver `Orbit::pitch`):
    // `pitch = pi/2` poe a camera a prumo sobre o alvo, e o deslocamento
    // horizontal zera.
    const float cp = std::cos(orbit.pitch);
    const float sp = std::sin(orbit.pitch);
    const float cy = std::cos(orbit.yaw);
    const float sy = std::sin(orbit.yaw);

    return { orbit.target.x + cp * sy * orbit.distance, orbit.target.y + sp * orbit.distance,
             orbit.target.z + cp * cy * orbit.distance };
}

Mat4 view(const Orbit& orbit)
{
    const Vec3 origem = eye(orbit);

    // Base DESTRA com a camera olhando para -Z: `f` aponta do olho para o alvo,
    // e a terceira coluna guarda `-f`.
    const Vec3 f = normalize(sub(orbit.target, origem));

    // O `up` de referencia e +Y. Camera a prumo (`pitch = +-pi/2`) deixa `f`
    // paralelo a ele e o produto vetorial degenera.
    //
    // **O que se poe no lugar NAO e livre: o limite existe e depende do yaw.**
    // Fora do polo, `cross(f, +Y)` normalizado vale `(cos yaw, 0, -sin yaw)`
    // (com `cos(pitch) > 0`) -- entao e esse o valor que faz a base continuar
    // continua quando o pitch chega a pi/2.
    //
    // A versao ate a 0.16.0 usava uma constante (`cross(f, -Z)`), que IGNORA o
    // giro: a um pitch de 89,99 graus a imagem dependia do yaw, e exatamente a
    // 90 ela saltava para uma orientacao fixa. O comentario de la afirmava o
    // contrario do que o codigo fazia.
    Vec3 r = cross(f, Vec3{ 0.0f, 1.0f, 0.0f });
    if (dot(r, r) <= 1e-12f)
    {
        r = { std::cos(orbit.yaw), 0.0f, -std::sin(orbit.yaw) };
    }
    r = normalize(r);
    const Vec3 u = cross(r, f);

    Mat4 v = Mat4::identity();
    at(v, 0, 0) = r.x;
    at(v, 1, 0) = r.y;
    at(v, 2, 0) = r.z;
    at(v, 0, 1) = u.x;
    at(v, 1, 1) = u.y;
    at(v, 2, 1) = u.z;
    at(v, 0, 2) = -f.x;
    at(v, 1, 2) = -f.y;
    at(v, 2, 2) = -f.z;
    at(v, 3, 0) = -dot(r, origem);
    at(v, 3, 1) = -dot(u, origem);
    at(v, 3, 2) = dot(f, origem);
    return v;
}

bool degenerada(const Mat4& m)
{
    for (const float v: m.m)
    {
        if (v != 0.0f)
        {
            return false;
        }
    }
    return true;
}

Mat4 orthographic(const float halfWidth, const float aspect, const float nearPlane, const float farPlane)
{
    // Entrada degenerada devolve a matriz ZERADA -- ver `degenerada()` no
    // cabecalho. Ate a 0.16.0 so o `aspect` era conferido, e a saida dele era um
    // valor plausivel (`halfHeight = halfWidth`); os outros dois divisores nao
    // tinham guarda nenhuma e emitiam `inf`.
    if (halfWidth <= 0.0f || aspect <= 1e-8f || farPlane <= nearPlane)
    {
        return Mat4{};
    }

    const float halfHeight = halfWidth / aspect;

    Mat4 p = Mat4::identity();
    at(p, 0, 0) = 1.0f / halfWidth;
    at(p, 1, 1) = 1.0f / halfHeight;
    // Destra com profundidade 0..1: o -Z da vista cresce para longe, e
    // `near -> 0`, `far -> 1`.
    at(p, 2, 2) = -1.0f / (farPlane - nearPlane);
    at(p, 3, 2) = -nearPlane / (farPlane - nearPlane);
    return p;
}

Mat4 perspective(const float fovY, const float aspect, const float nearPlane, const float farPlane)
{
    // Mesma regra da ortografica. O `fovY` tem de estar em (0, pi): fora disso a
    // tangente da meia abertura e zero, negativa ou infinita, e nenhuma das tres
    // e uma camera.
    if (fovY <= 1e-6f || fovY >= kPi - 1e-6f || aspect <= 1e-8f || nearPlane <= 0.0f || farPlane <= nearPlane)
    {
        return Mat4{};
    }

    const float f = 1.0f / std::tan(fovY * 0.5f);

    Mat4 p;
    at(p, 0, 0) = f / aspect;
    at(p, 1, 1) = f;
    at(p, 2, 2) = farPlane / (nearPlane - farPlane);
    at(p, 2, 3) = -1.0f;
    at(p, 3, 2) = (nearPlane * farPlane) / (nearPlane - farPlane);
    return p;
}

Mat4 multiply(const Mat4& a, const Mat4& b)
{
    Mat4 r;
    for (int col = 0; col < 4; ++col)
    {
        for (int row = 0; row < 4; ++row)
        {
            float soma = 0.0f;
            for (int k = 0; k < 4; ++k)
            {
                soma += at(a, k, row) * at(b, col, k);
            }
            at(r, col, row) = soma;
        }
    }
    return r;
}

bool unproject(const Mat4& viewProj, const float screenX, const float screenY, const float viewportW,
               const float viewportH, const float planeY, Vec3& outWorld)
{
    if (viewportW <= 0.0f || viewportH <= 0.0f)
    {
        return false;
    }

    // Pixel -> NDC. O `1 - 2*y/h` e a inversao do eixo vertical: a tela cresce
    // para BAIXO e o NDC cresce para CIMA. Acontece uma vez, aqui.
    const float ndcX = 2.0f * screenX / viewportW - 1.0f;
    const float ndcY = 1.0f - 2.0f * screenY / viewportH;

    // Dois pontos do raio: o do plano proximo (z=0) e o do distante (z=1). Vale
    // para ortografica e perspectiva sem caso especial, porque desprojetar os
    // dois e depois dividir por `w` cobre as duas.
    Mat4 inv;
    {
        // Inversa geral 4x4 por cofatores. Uma matriz de camera nunca e
        // singular na pratica, mas a checagem existe porque o custo e uma
        // comparacao e a alternativa e devolver NaN silencioso.
        const float* m = viewProj.m;
        float        a[16];
        a[0] = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] +
               m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
        a[4] = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] -
               m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
        a[8] = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] +
               m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
        a[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] -
                m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
        a[1] = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] -
               m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
        a[5] = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] +
               m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
        a[9] = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] -
               m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
        a[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] +
                m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
        a[2] = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] +
               m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
        a[6] = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] -
               m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
        a[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] +
                m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
        a[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] -
                m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
        a[3] = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] -
               m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
        a[7] = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] +
               m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
        a[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] -
                m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
        a[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] +
                m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

        const float det = m[0] * a[0] + m[1] * a[4] + m[2] * a[8] + m[3] * a[12];
        if (std::fabs(det) <= 1e-12f)
        {
            return false;
        }
        const float invDet = 1.0f / det;
        for (int i = 0; i < 16; ++i)
        {
            inv.m[i] = a[i] * invDet;
        }
    }

    const auto desprojetar = [&inv](const float x, const float y, const float z, Vec3& out) {
        const float wx = inv.m[0] * x + inv.m[4] * y + inv.m[8] * z + inv.m[12];
        const float wy = inv.m[1] * x + inv.m[5] * y + inv.m[9] * z + inv.m[13];
        const float wz = inv.m[2] * x + inv.m[6] * y + inv.m[10] * z + inv.m[14];
        const float ww = inv.m[3] * x + inv.m[7] * y + inv.m[11] * z + inv.m[15];
        if (std::fabs(ww) <= 1e-12f)
        {
            return false;
        }
        out = { wx / ww, wy / ww, wz / ww };
        return true;
    };

    Vec3 perto, longe;
    if (!desprojetar(ndcX, ndcY, 0.0f, perto) || !desprojetar(ndcX, ndcY, 1.0f, longe))
    {
        return false;
    }

    // Interseccao do raio com o plano horizontal `y = planeY`.
    const float dy = longe.y - perto.y;
    if (std::fabs(dy) <= 1e-8f)
    {
        // O raio e paralelo ao chao: nao ha ponto, e inventar um seria pior do
        // que dizer que nao ha. E o mesmo caso que o vigil tratava devolvendo o
        // centro — aqui a resposta e "nao", e quem chama decide.
        return false;
    }

    const float t = (planeY - perto.y) / dy;

    // **`t < 0` e o plano ATRAS da camera**, e ele tem de ser um `false` como
    // qualquer outro "nao ha resposta". O raio vai do plano proximo (t=0) para o
    // distante (t=1); um `t` negativo quer dizer que o encontro fica atras do
    // olho -- e o caso de mirar ACIMA do horizonte, em perspectiva, ou de pedir
    // um plano mais alto do que a camera.
    //
    // Ate a 0.16.0 isto devolvia `true` com um ponto que existe, esta no plano
    // pedido, e fica do lado de la de quem esta olhando: um alvo plausivel para
    // um clique que nao acertou nada.
    if (t < 0.0f)
    {
        return false;
    }

    outWorld = { perto.x + (longe.x - perto.x) * t, planeY, perto.z + (longe.z - perto.z) * t };
    return true;
}

} // namespace cengine::camera3d
