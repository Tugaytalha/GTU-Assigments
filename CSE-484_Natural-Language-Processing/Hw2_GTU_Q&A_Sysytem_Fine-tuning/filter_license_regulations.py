import os
import shutil

def copy_files_to_data_directory(file_list):
    """
    Copies specific files to the ./data/ directory based on the provided information.

    Args:
        file_list (list): A list of file paths available.
    """

    # Create the ./data/ directory if it doesn't exist
    data_dir = "./data/"
    if not os.path.exists(data_dir):
        os.makedirs(data_dir)

    # Dictionary mapping directive numbers to file names
    directive_mapping = {
        "YÖ-0005": "YÖ-0005_Dikey_Geçiş_Lisans_Uygulama_Yönergesi R1.pdf",
        "YÖ-0032": "YÖ-0032_Prof._Dr._Nejat_Goyunc_Kutuphanesi_Hizmetlerinden_Yararlanma_Yonergesi R1.pdf",
        "YÖ-0004": "YÖ-0004 Çift Anadal Programı Yönergesi R2.pdf",
        "YÖ-0002": "YÖ-0002 Önlisans-Lisans İngilizce Hazırlık Eğitim-Öğretim ve Sınav Yönergesi R7.pdf",
        "YÖ-0012": "YÖ-0012 Yandal Programı Yönergesi R3.pdf",
        "YÖ-0013": "YÖ-0013 Önlisans-Lisans Programları Yatay Geçiş Yönergesi R3.pdf",
        "YÖ-0024": "YÖ-0024 Öğrenci Toplulukları Kuruluş ve İşleyiş Yönergesi R4.pdf",
        "YÖ-0011": "YÖ-0011 Uluslararası Öğrencilerin Lisans Programlarına Başvuru, Kabul ve Kayıt Yönergesi R7.pdf",
        "YÖ-0016": "YÖ-0016 Mimari Tasarım Dersleri Uygulama Esasları R2.pdf",
        "YÖ-0017": "YÖ-0017 Mimari Tasarm VIII Dersi Uygulama Esasları R2.pdf",
        "YÖ-0018": "YO-0018_Mimarlk_Fakultesi_Lisans_Eitimi_Staj_Yonergesi_R2.pdf", # Assuming YÖ-0018 corresponds to the closest numbered file
        "YÖ-0003": "YO-0003_Dereceye_Giren_Lisans_Mezunlarn_Tespitine_likin_Yonerge_R1.pdf",
        "YÖ-0042": "YÖ-0042 Erasmus Öğrenci ve Personel Değişim Yönergesi R3.pdf",
        "YÖ-0006": "YÖ-0006 Mezuniyet Belgesi İle Diploma ve Diploma Defterinin Düzenlenmesinde Uyulacak Esaslara İlişkin Yönerge R2.pdf",
        "YÖ-0007": "YÖ-0007 Lisans Muafiyet ve İntibak Yönergesi R2.pdf",
        "YÖ-0035": "YÖ-0035_Öğrenci_Konseyi_Yonergesi R1.pdf",
        "YÖ-0071": "YO-0071_Temel_Bilimler_Fakultesi_Lisans_Eitimi_Staj_Yonergesi_R0.pdf",
    }

    # Copy the files
    for directive, filename in directive_mapping.items():
        source_path = None
        for file_path in file_list:
          if filename in file_path:
            source_path = file_path
            break

        if source_path:
            destination_path = os.path.join(data_dir, filename)
            shutil.copy2(source_path, destination_path)  # copy2 preserves metadata
            print(f"Copied '{filename}' to '{data_dir}'")
        else:
            print(f"Warning: File '{filename}' not found in the provided list.")

# Assuming you're in the same directory as the files
file_list = [
    "YO-0003_Dereceye_Giren_Lisans_Mezunlarn_Tespitine_likin_Yonerge_R1.pdf",
    "YO-0018_Mimarlk_Fakultesi_Lisans_Eitimi_Staj_Yonergesi_R2.pdf",
    "YO-0020_Fikri_Sinai_Mulkiyet_Haklar_Yonergesi_R2.pdf",
    "YO-0021_Teknoloji_Transfer_Ofisi_Yonergesi_R1.pdf",
    "YO-0023_Misafirhane_Yonergesi_R1.pdf",
    "YO-0025_Ksmi_Zamanl_Orenci_Caltrma_Yonergesi_R2.pdf",
    "YO-0036_Okul_Oncesi_Egitim_Merkezi_Kurulus_ve_Calsma_Yonergesi_R1.pdf",
    "YO-0045_AYDEK_Calma_Usul_ve_Esaslar_R3.pdf",
    "YO-0046_Bisiklet_Kullanm_Kurallar_Yonergesi_R1.pdf",
    "YO-0047_Di_Hekimlii_Hizmetleri_Yonergesi_R1.pdf",
    "YO-0049_Guvenlik_ve_Trafik_Uygulamalar_Yonergesi_R0.pdf",
    "YO-0050_Mevzuat_Komisyonu_Yonergesi_R0.pdf",
    "YO-0051_Psikolojik_Danmanlk_ve_Rehberlik_Hizmetleri_Yonergesi_R1.pdf",
    "YO-0052_Surekli_Eitim_Uygulama_ve_Aratrma_Merkezi_Eitim_ve_Sertifika_Programlar_Yonergesi_R0.pdf",
    "YO-0053_Arabuluculuk_Komisyonu_Calma_Usul_ve_Esaslar_R0.pdf",
    "YO-0056_Dr._Oretim_Uyelerinin_Gorev_Surelerinin_Uzatlmas_cin_Belirlenen_Koullar_R0.pdf",
    "YO-0059_Aselsan_Akademi_Lisansustu_Eitim_Program_Uygulama_Esaslar_R0.pdf",
    "YO-0061_dari_Personelin_l_D_Naklen_Tayin_lemleri_Hakkndaki_Usul_ve_Esaslar_R0.pdf",
    "YO-0065_Guvenlik_Soruturmas_ve_Ariv_Aratrmas_Yonergesi_R0.pdf",
    "YO-0067_Akademik_Ariv_ve_Ack_Eriim_Yonergesi_R0.pdf",
    "YO-0071_Temel_Bilimler_Fakultesi_Lisans_Eitimi_Staj_Yonergesi_R0.pdf",
    "YÖ-0001 Engelsiz GTÜ Birimi Yönergesi R2.pdf",
    "YÖ-0002 Önlisans-Lisans İngilizce Hazırlık Eğitim-Öğretim ve Sınav Yönergesi R7.pdf",
    "YÖ-0004 Çift Anadal Programı Yönergesi R2.pdf",
    "YÖ-0005_Dikey_Geçiş_Lisans_Uygulama_Yönergesi R1.pdf",
    "YÖ-0006 Mezuniyet Belgesi İle Diploma ve Diploma Defterinin Düzenlenmesinde Uyulacak Esaslara İlişkin Yönerge R2.pdf",
    "YÖ-0007 Lisans Muafiyet ve İntibak Yönergesi R2.pdf",
    "YÖ-0008_Lisans_Ustu_Yabanc_Ogrenci_Yonergesi R1.pdf",
    "YÖ-0009 Lisansüstü Ingilizce Hazırlık Egitim Ögretim ve Sınav Yönergesi R2.pdf",
    "YÖ-0011 Uluslararası Öğrencilerin Lisans Programlarına Başvuru, Kabul ve Kayıt Yönergesi R7.pdf",
    "YÖ-0012 Yandal Programı Yönergesi R3.pdf",
    "YÖ-0013 Önlisans-Lisans Programları Yatay Geçiş Yönergesi R3.pdf",
    "YÖ-0014_Etik_İkeleri_ve_Etik_Kurulu_Yönergesi R2.pdf",
    "YÖ-0015_İnsan_Araştırmaları_Etik_Kurul_Yönergesi R1.pdf",
    "YÖ-0016 Mimari Tasarım Dersleri Uygulama Esasları R2.pdf",
    "YÖ-0017 Mimari Tasarm VIII Dersi Uygulama Esasları R2.pdf",
    "YÖ-0019 Mühendislik Fakültesi Staj Yönergesi R4.pdf",
    "YÖ-0022 Psikolojik Taciz İle Mücadele Birimi Yönergesi R2.pdf",
    "YÖ-0024 Öğrenci Toplulukları Kuruluş ve İşleyiş Yönergesi R4.pdf",
    "YÖ-0028 İşletme Fakültesi Staj Yönergesi-R3.pdf",
    "YÖ-0030 Yazışma Usulleri, İmza Yetkileri ve Yetki Devri Yönergesi R5.pdf",
    "YÖ-0031_İç_Denetim_Yonergesi R1.pdf",
    "YÖ-0032_Prof._Dr._Nejat_Goyunc_Kutuphanesi_Hizmetlerinden_Yararlanma_Yonergesi R1.pdf",
    "YÖ-0034_Kurtarma_Ekibi_Yonergesi R1.pdf",
    "YÖ-0035_Öğrenci_Konseyi_Yonergesi R1.pdf",
    "YÖ-0037_Turkce_Hazrlk_Ogretim_ve_Snav_Yonergesi R1 - Copy 1.pdf",
    "YÖ-0038_Yabanc_Dil_Egitim_Programlar_ve_Kurslar_Yonergesi R1.pdf",
    "YÖ-0039_Yurt_Idare_ve_Isletme_Yonergesi R1.pdf",
    "YÖ-0042 Erasmus Öğrenci ve Personel Değişim Yönergesi R3.pdf",
    "YÖ-0043 Bilimsel Araştırma Projeleri Uygulama Yönergesi R7.pdf",
    "YÖ-0048 Endüstriyel Uygulamalar Dersi Yönergesi R1.pdf",
    "YÖ-0054 Lisansüstü Eğitim Öğretim Yönetmeliği Senato Uygulama Esasları R4.pdf",
    "YÖ-0055 GTÜ Akademik Yükseltme ve Atama Koşulları Yönergesi-R9.pdf",
    "YÖ-0057 Araştırma Görevlilerinin Görev Sürelerinin Uzatılması Koşulları Yönergesi R2.pdf",
    "YÖ-0058 Personel Ödül Esasları Yönergesi R5.pdf",
    "YÖ-0062 Araştırma Planlama ve Danışma Kurulu Yönergesi R1.pdf",
    "YÖ-0063 Yabancı Uyruklu Sözleşmeli Akademik Personel Çalıştırma Yönergesi R3.pdf",
    "YÖ-0064 Bilişim Politikaları Yönergesi R1.pdf",
    "YÖ-0066 Önlisans-Lisans Öğrenci Danışmanlığı Yönergesi R2.pdf",
    "YÖ-0068_Hayvan Deneyleri Yerel Etik Kurulu (HADYEK) R2.pdf",
    "YÖ-0069 Misafir, Doktora Sonrası Araştırmacı ve Kısmi Zamanlı Araştırmacı Yönergesi R0.pdf",
    "YÖ-0070 Hizmet İçi Eğitim Yönergesi R1.pdf",
    "YÖ-0072 Atık Yönetimi Yönergesi R1.pdf",
    "YÖ-0073 Gebze Teknik Üniversitesi Spor Tesisleri İşletme Yönergesi R0.pdf",
    "YÖ-0074 Uluslararası Bilimsel Yayınları Teşvik Programı ve Uygulama Esasları R0.pdf",
    "YÖ-0075 Yayın Yönergesi R0.pdf",
    "YÖ-0076 Yemek Servisi Yürütme Kurulunun Kuruluş ve Görevlerine İlişkin Yönerge R1.pdf",
    "YÖ-0077 Emeklilik Yaş Haddini Doldurmuş Öğretim Üyelerinin Sözleşmeli Olarak Çalıştırılmasına İlişkin Usul ve Esaslar R0.pdf",
    "YÖ-0078 Azami Öğrenim Süresini Tamamlayan Ancak Mezun Olamayan Önlisans-Lisans Öğrencileri İçin Uygulama Usul ve Esasları R2.pdf",
    "YÖ-0079 Biyomühendislik Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0080 Elektronik Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0081 İnşaat Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0082 Harita Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0083 Kimya Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0084 Makine Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0085 Çevre Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0086 Endüstri Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0087 Bilgisayar Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0088 Malzeme Bilimi ve Mühendisliği Bölümü Staj Uygulama Esasları R1.pdf",
    "YÖ-0089 Havacılık ve Uzay Bilimleri Fakültesi Lisans Eğitimi Staj Yönergesi R0.pdf",
    "YÖ-0091 Araştırma Geliştirme Komisyonu Yönergesi R0.pdf",
    "YÖ-0092 Eğitim-Öğretim Komisyonu Yönergesi R1.pdf",
    "YÖ-0093 Endüstriyel Uygulamalar Dersi Yönergesi - Yeni R0.pdf",
    "YÖ-0094 GTÜ 101 Müfredat Dışı Etkinlik Uygulama Esasları R1.pdf",
    "YÖ-0095 Sürdürülebilirlik Ofisi Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0096 Mezunlar Ofisi Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0097 Endüstriyel Hizmetler Ofisi Koordinatörlüğü (GebzeLAB) Yönergesi R0.pdf",
    "YÖ-0098 İş Sağlığı ve Güvenliği İç Yönergesi R0.pdf",
    "YÖ-0099 Toplumsal Katkı Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0100 GTU 110 Bilimsel ve Teknolojik Etkinlik Dersi Uygulama Esasları R0.pdf",
    "YÖ-0101 Sağlıklı Kampüs-Sahipsiz Hayvanlar Yönergesi R0.pdf",
    "YÖ-0102 Dumansız Hava Sahası Yönergesi R0.pdf",
    "YÖ-0103 Araştırma Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0104 Ayrımcılıkla Mücadele ve Fırsat Eşitliği Yönergesi R0.pdf",
    "YÖ-0105 Bağımlılıkla Mücadele Komisyonu Yönergesi R0.pdf",
    "YÖ-0106 Derecelendirme Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0107 Ulusal ve Uluslararası Danışma Kurulları Yönergesi R0.pdf",
    "YÖ-0108 Veri ve Dijital Dönüşüm Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0109 Uluslararası İlişkiler Koordinatörlüğü Yönergesi R0.pdf",
    "YÖ-0110 SGM 216 Mesleki Eğitim Dersi Yönergesi R0.pdf"
]

copy_files_to_data_directory(file_list)

