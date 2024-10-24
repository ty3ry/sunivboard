# Allwinner F1C100/200s Linux Board

## Firmware Build 
1. persiapkan host machine untuk proses build dengan OS Linux (Ubuntu)
2. clone repository
    ```bash
    $ git clone https://github.com/ty3ry/sunivboard.git
    $ git submodule update --init
    ```
3. pindah ke directory buildroot dan konfigurasi external tree untuk custom package dan konfigurasi yang berkaitan dengan board 
    ```bash
    $ cd buildroot
    $ make BR2_EXTERNAL=../ allwinner_suniv_16m_defconfig
    ```
    atau jika ingin hasil build berada pada direktori tertentu dapat menggunakan opsi ``O`` alias output
   ```bash
   $ make BR2_EXTERNAL=../ allwinner_suniv_16m_defconfig O=<output_direktori>
   ```
4. mulai proses build
    ```bash
    $ make
    ```
5. ketika sudah selesai maka hasil dari proses build akan berada di lokasi directory ``<output_direktori>/output/images/flash.bin``
6. untuk melakukan proses flashing ke board target maka dapat menggunakan sunxi tool yang berada di directory ``output/host/bin/``
7. jika proses build berada pada machine yang berbeda (dilakukan secara remote) sunxi tools harus diinstalasi pada machine lokal. untuk instalasi dapat dilihat pada https://github.com/thirtythreeforty/sunxi-tools dan lakukan build secara manual agar dapat menggunakan nya pada machine lokal.
proses build sunxi-tools pada experiment ini dilakukan pada board ``Orangepi-pc`` dengan OS Armbian (Ubuntu 20.04 LTS armv7l).
proses build sunxi-tools:
    ```bash
    $ git clone https://github.com/thirtythreeforty/sunxi-tools.git
    $ cd sunxi-tools
    $ make tools
    $ make install
    ```
    untuk build pada host dengan OS windows masih belum bisa dilakukan (TODO).
8. proses flash dilakukan dengan menggunakan perintah berikut :
    *note: pastikan bahwa target sudah masuk ke dalam ``FEL mode``*
    untuk masuk ke dalam mode FEL, langkah-langkah nya adalah sebagai berikut :
    - short kan pin 1 dan 4 dari IC SPI flash.
    - power on modul dengan memasukkan konektor USB ke modul.
    - unshort pin 1 dan 4
    - eksekusi command di bawah ini

    ```bash
    $ output/host/bin/sunxi-fel -p spiflash-write 0 output/images/flash.bin
    ```
    - ketika gagal maka ulangi proses di atas sampai muncul progress bar dari tool sunxi-fel yang menyatakan bahwa proses flashing sedang berlangsung.
