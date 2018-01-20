Mod Loader
==================

## O que é?

Mod Loader é um plugin ASI para o Grand Theft Auto San Andreas que adiciona uma forma extremamente fácil e amigável de se instalar e desinstalar modificações no jogo, sem tocar em qualquer arquivo do mesmo.

A utilização é simples: você só precisa criar uma ou mais pastas dentro do diretório modloader/ e então jogar o conteúdo das modificações lá e pronto!
É recomendado que se tenha uma pasta para cada modificação, por questão de organização.

## Instalando o Mod Loader

### Instalando no GTA San Andreas

  Mod Loader requer um [ASI Loader](http://www.gtagarage.com/mods/show.php?id=21709), tenha certeza de que você tenha um.

  Em seguida, extraia o *modloader.asi* e a pasta *modloader* para o diretório de seu jogo.

### Instalando no GTA Vice City ou GTA III:

  Mod Loader requer o [Ultimate ASI Loader](https://github.com/ThirteenAG/Ultimate-ASI-Loader/releases), tenha certeza de que você tenha um.

  Em seguida, extraia o *modloader.asi* para pasta *scripts/* e a pasta *modloader* para o diretório de seu jogo.

## Instalando Mods no Mod Loader

Para instalar um mod no Mod Loader, é extremamente simples, apenas extraia o conteúdo do seu mod **dentro de uma pasta** no diretório do *modloader*.

Isso significa que os seguintes são métodos válidos de instalação:

 + modloader/nsx/infernus.dff
 + modloader/nsx/outra pasta/infernus.dff

Entretanto, o seguinte **NÃO** é um método válido:

 - modloader/infernus.dff 
 - modloader/.data/infernus.dff


## Desinstalando Mods do Mod Loader

Ainda mais fácil, apenas delete o conteúdo do mod que deseja instalar do diretório do *modloader*.
Se você só quiser desabilitar o mod por um instante, vá para o menu in-game para desabilitá-lo, ou edite o *modloader.ini* manualmente.

## Destaques

- Não substitua **NENHUM** arquivo original. Nunca. Sério.
- Deixe que o Mod Loader tome conta de tudo.
    + Mixagem de arquivos data
        * Portanto você pode, por exemplo, ter 70 arquivos handling.cfg no modloader e eles irão funcionar perfeitamente.
    + Leitura de arquivos 'leia-me'
        * Não é necessário ligar para as linhas data em arquivos 'leia-me', o Mod Loader instala elas por você também !!!
- Recarregar Mods
    + Mude ou adiciona arquivos enquanto o jogo está rodando e veja-as imediatamente!!!
- Suporte para linhas de comando
    + Veja *modloader/.data/Command Line Arguments.md*
- Menu 'em jogo' para configurações
    + Vá para *Options > Mod Configuration*
    + Quando o menu não estiver disponível, faça as edições manualmente em *modloader/modloader.ini* e *modloader/.data/config.ini*
- Profiles, assim você pode ter varios jogos em um.
    + Leia *modloader/.data/Profiles.md* for para detalhes

### Encontrou uma falha?

É essencial que você reporte bugs, para que o Mod Loader seja melhorado. Para reportar um bug, vá para um dos canais a seguir:

 * No GitHub, usando [nosso issue tracker](https://github.com/thelink2012/sa-modloader/issues)
 * Suporte em inglês na [GTA Forums](http://gtaforums.com/topic/669520-sarel-mod-loader/)
 * Suporte em português na [Brazilian Modding Studio Forums](http://brmodstudio.forumeiros.com/t3591-mod-loader-topico-oficial)

Ao reportar um bug, **POR FAVOR**, forneça o arquivo *modloader/modloader.log* criado logo após o crash e dê informações detalhadas de como reproduzir o bug.

### Executáveis suportados

Nem todos os executáveis são suportados no momento. Os suportados são:

 + GTA III 1.0
 + GTA VC 1.0
 + GTA SA 1.0 US
 + GTA SA 1.0 EU

## Download

Você pode fazer o download da última versão do Mod Loader em um dos seguintes links:

 * [GTA Garage](http://www.gtagarage.com/mods/show.php?id=25377), para a última versão estável.
 * [GitHub](https://github.com/thelink2012/modloader/releases), para a última versão (incluindo não-estáveis).

## Código-Fonte

Mod Loader é um projeto open source, sinta-se livre para aprender e contribuir.
O código fonte está sob a MIT License, se ligue no [GitHub](https://github.com/thelink2012/modloader/).

## Créditos

Finalmente, créditos.

#### Desenvolvedor
  * LINK/2012 (<dma_2012@hotmail.com>)

#### Muito Obrigado à
  * ArtututuVidor$, Andryo, Junior_Djjr e JNRois12 por alpha-testarem
  * Gramps e TJGM pelo suporte emocional.
  * SilentPL por muitos fixes e ajudas no desenvolvimento do projeto.
  * ThirteenAG por me fornecer varios ponteiros (literalmente) para a versão do GTA III / Vice City.

