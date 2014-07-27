Mod Loader
==================

## O que é?

Mod Loader é um plugin ASI para o Grand Theft Auto San Andreas que adiciona uma forma extremamente facil e amigavel de se instalar e desinstalar modificações no jogo, sem tocar em qualquer arquivo da sua instalação.

A utilização é simples, você só precisa criar uma ou mais pastas dentro do directorio modloader/ e então jogar o conteudo dos mods lá. Pronto!
É recomendado que se tenha uma pasta para cada modificação, por questão de organização.

## Installing Mod Loader

Mod Loader requer um [ASI Loader](http://www.gtagarage.com/mods/show.php?id=21709), tenha certeza que você tem um.

Em seguida, é so extrair o *modloader.asi* e a pasta *modloader* para o directorio do seu jogo.

## Instalando Mods no Mod Loader

Para instalar um mod no Mod Loader, é extremamente simples, apenas extraia o conteudo do seu mod **dentro de uma pasta** no directorio do *modloader*.

Isso significa que o seguinte são metodos validos de instalação:

 + modloader/nsx/infernus.dff
 + modloader/nsx/another folder/infernus.dff

Mas o seguinte **NÃO** é um metodo valido:

 - modloader/infernus.dff 
 - modloader/.data/infernus.dff


## Desinstalando Mods do Mod Loader

Ainda mais facil, apenas delete o conteudo do mod que deseja instalar da directorio do *modloader*.
Se você só quer desabilitar o mod por um instante, vá para o menu in-game para desabilita-lo ou edite o *modloader.ini* manualmente.

## Highlights and Detalhes

- Não substitua **NENHUM** arquivo original, nunca, sério.
- Deixe o Mod Loader tomar conta de tudo
    + Mixagem de arquivos data
        * Portanto você pode por exemplo ter 70 arquivos handling.cfg no modloader e eles vão funcionar perfeitamente.
    + Leitura de arquivos leia-me
        * Não precisa ligar para linhas data em arquivos leia-me, o Mod Loader instala elas por você tambem!!
- Recarregue os mods
    + Mude ou adiciona arquivos enquanto o jogo esta rodando e veja as mudanças imediatamente*!!!!
- Suporte para linhas de comando
    + Veja *modloader/.data/Command Line Arguments.md*
- Menu In-Game para configurações
    + Vá para *Options > Mod Loader Setup*
    + Quando o menu não estiver disponivel, faça as edições manualmente em *modloader/modloader.ini* e *modloader/.data/config.ini*

\* No momento você precisa apertar F4 para recarregar os mods, alguns arquivos não são recarregaveis, mas a maioria é.



## Em Progresso

Mod Loader ainda esta em progresso, portanto nem tudo é carregavel ainda.
Isso é, alguns arquivos *.dat* não são carregaveis por enquanto, mas estamos trabalhando nisso.

### Encontrou um bug?

É essencial que você reporte bugs, para que o loader seja melhorado, para reportar um bug vá para um dos canais a seguir:

 * No GitHub, usando [nosso issue tracker](https://github.com/thelink2012/sa-modloader/issues)
 * Suporte em inglês na [GTA Forums](http://gtaforums.com/topic/669520-sarel-mod-loader/)
 * Suporte em português na [Brazilian Modding Studio Forums](http://brmodstudio.forumeiros.com/t3591-mod-loader-topico-oficial)

Quando tiver reportando o bug, **POR FAVOR, PELO AMOR DE DEUS**, forneça o arquivo *modloader/modloader.log* criado logo apos o crash e dé informações detalhadas de como reproduzir o bug.

### Executaveis suportados

Nem todos os executaveis são suportados no momento, os suportados são:

 + 1.0 US *(Original, HOODLUM, Listener's Compact)*

## Download

Você pode fazer o download da ultima versão do Mod Loader em um dos seguintes links:

 * [GTA Garage](http://www.gtagarage.com/mods/show.php?id=25377), para a ultima versão estavel.
 * [GitHub](https://github.com/thelink2012/modloader/releases), para a ultima versão (incluindo não-estaveis).

## Codigo-Fonte

Mod Loader é um projeto open source, se sinta livre para aprender e contribuir.
O codigo fonte esta sobre a GPL v3, se ligue no [GitHub](https://github.com/thelink2012/modloader/).

## Creditos

Finalmente, creditos.

#### Desenvolvedor
  * LINK/2012 (<dma_2012@hotmail.com>)

#### Muito Obrigado Para
  * ArtututuVidor$, Andryo, Junior_Djjr, JNRois12 por alpha-testarem
  * methodunderg and TheJamesGM pelo seu suporte emocional.
  * SilentPL pelo seu suporte adicional
