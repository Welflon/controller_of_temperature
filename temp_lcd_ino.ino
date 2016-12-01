/*
Program służy do automatycznego zacierania słodu w piwowarstwie domowym 
Program został napisany przez Welflon ~~ Student I roku informatyki na UZ */

/* 
    NOTATKI

*/
#include <LiquidCrystal.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define port_termometru 8
#define ilosc_pomiarow 4
// ^ przewidziana parzysta liczba 
#define buzzer_port  9
#define ssr_port  10
const float time_stage[] = {10000.00,18000.00,30000.00};
//^czasy w milisekundach kolejno pierwszy, drugi, trzeci etap
const  float temperature_stage[] = {20.00,64.00,72.00};
// kolejno temperaturty etapów 52, 64, 72

 OneWire oneWire(port_termometru);
 DallasTemperature czujnik(&oneWire);
 LiquidCrystal lcd(2,3,4,5,6,7);

class Termometr {  
        float mediana;           
  public:   
    Termometr(){mediana = 2.0;}
    friend class Wyswietlacz_lcd;      
    void sortuj(float tablica_sortowana[], float tablica_POsortowana[]){
              int licznik_obejsc_petli = 0;
              int pomocnicza_do_zamiany_w_tablicy = 0;
 
              for(int licznik = 0; licznik < ilosc_pomiarow; licznik++){
                      licznik_obejsc_petli = licznik;
                      tablica_POsortowana[licznik] = tablica_sortowana[licznik];
                      while(licznik>0){
                                     if(tablica_POsortowana[licznik]<tablica_POsortowana[licznik-1]){
                                            pomocnicza_do_zamiany_w_tablicy = tablica_POsortowana[licznik-1];
                                            tablica_POsortowana[licznik-1] = tablica_POsortowana[licznik];
                                            tablica_POsortowana[licznik] = pomocnicza_do_zamiany_w_tablicy;
                                      };
                          licznik--;
                      };
                      licznik = licznik_obejsc_petli;
            };  
          }; 
    float pomiar_temperatury(){
          float temp = 0.0;
          czujnik.requestTemperatures(); 
          temp = czujnik.getTempCByIndex(0);
          return temp;
          };  
    float licz_mediane(){                       
            float temperatury_zmierzone[ilosc_pomiarow-1];
            float temperatury_posortowane[ilosc_pomiarow-1];        
            for(int nr_pomiaru = 0; nr_pomiaru < ilosc_pomiarow ; nr_pomiaru++){
                 temperatury_zmierzone[nr_pomiaru] = pomiar_temperatury();    
           };

          sortuj(temperatury_zmierzone,temperatury_posortowane);

          mediana = ((temperatury_posortowane[ilosc_pomiarow/2] + temperatury_zmierzone[(ilosc_pomiarow/2)-1])/2.0);
          return mediana;
      }; 

     boolean sprawdz_temperature(float temperatura_stala, float temperatura_aktualna){
            if(temperatura_stala <= temperatura_aktualna) return true;
            else return false;
          };
};

class Wyswietlacz_lcd{
          float wynik;   
    public:
          Wyswietlacz_lcd(){wynik = 0.0;}                   
          void wyswietlaj_na_lcd(float zmienna_wyswietlana_float){     
                        lcd.setCursor(0,0);
                        lcd.print(zmienna_wyswietlana_float);          
                        lcd.setCursor(6,0);
                        lcd.print("*C");
                };
          void wyswietlaj_na_lcd_mediany(Termometr& obiekt){
                      wynik = obiekt.licz_mediane();       
                      wyswietlaj_na_lcd(wynik);
                };

          void wyswietl_na_lcd_czas(float czas, int piksel_poczatkowy){ 
                    czas = czas / 1000.0;
                    czas = czas / 60.0;
                    
                   lcd.setCursor(piksel_poczatkowy,1);  
                   lcd.print((int)czas);          
                   lcd.setCursor(piksel_poczatkowy+4,1);
                   
                   lcd.print("min");
              };            
};
class Buzzer {
public:
      void buzzuj(int ilosc_powtorzen){
                if(ilosc_powtorzen != 0) {                       
                    delay(1000);
                    digitalWrite(buzzer_port,HIGH);
                    delay(1000);
                    digitalWrite(buzzer_port,LOW);    
                    ilosc_powtorzen --;
                    buzzuj(ilosc_powtorzen);   
                };  
      };
};
class SSR {
public:
       Termometr obiekt_termometr;
       void stop_current(){
            digitalWrite(ssr_port,LOW);          
         };
       void start_current(){               
            digitalWrite(ssr_port,HIGH);    
         };
      void work_SSR(float stala_temperatura_etapu){
              if(stala_temperatura_etapu < obiekt_termometr.licz_mediane()){
                    start_current();
              }else stop_current();                      
          };  
};

class Czas{
        float time_start_stage = 0.0;
public:        
        float czas_dzialania_programu(){
                 float time_working_program;
                 time_working_program = (float)millis();
                 return time_working_program;
              };
         void set_time_start_stage(){
                time_start_stage = czas_dzialania_programu();
              };
        float czas_trwania_etapu(){
                 float real_time_stage = czas_dzialania_programu() - time_start_stage;
                 return real_time_stage;  
              };                
               
        float czas_do_konca_etapu( float max_time_stage){          
                 float time_to_end_stage;
                 time_to_end_stage = max_time_stage + time_start_stage - czas_dzialania_programu();  
                 return time_to_end_stage;      
              };
       float czas_poczatkowy_etapu(){
            return time_start_stage;          
       };

       void ustaw_czas_poczatkowy_jezeli(float stala){
             Termometr obiekt_termometr;
             Wyswietlacz_lcd obiekt_lcd;
             if(obiekt_termometr.sprawdz_temperature(stala,obiekt_termometr.licz_mediane())){
                set_time_start_stage();
                // temperatura osiąga wartość początkowa dla etapu - od tego momentu trwa etap
             } else {
                obiekt_lcd.wyswietlaj_na_lcd_mediany(obiekt_termometr);
                ustaw_czas_poczatkowy_jezeli(stala);
             }
       };
};

class Etap { 
private:
        int nr_etapu;
public:
      Termometr obiekt_termometr;
      Wyswietlacz_lcd obiekt_lcd;
      Buzzer obiekt_buzzer;
      SSR obiekt_SSR;
      Czas obiekt_czas;
      
      void set_nr_etapu(int liczba){
        nr_etapu = liczba;
     };
      virtual void wypisz_nr_etapu(){};
      
      void wyswietlanie_temperatury(){
          obiekt_lcd.wyswietlaj_na_lcd_mediany(obiekt_termometr);
      };
      
      void dzialanie_buzzer(){
          obiekt_buzzer.buzzuj(nr_etapu);  
      };
      
      void dzialanie_SSR(){
          obiekt_SSR.work_SSR(temperature_stage[nr_etapu-1]);
      
      };
      void wyswietl_czas_do_konca(){
          obiekt_lcd.wyswietl_na_lcd_czas(obiekt_czas.czas_do_konca_etapu(time_stage[nr_etapu-1] ),8);
      };
      
      
};

class Etap_1 : public Etap {
public:   
        virtual void wypisz_nr_etapu(){
                    lcd.setCursor(10,0);
                    lcd.print("Etap 1");        
        };
        
        Etap_1(){
          set_nr_etapu(1);
          obiekt_czas.set_time_start_stage();  
      }
};

class Etap_2 : public Etap {
public:
        virtual void wypisz_nr_etapu(){
                     lcd.setCursor(10,0);
                     lcd.print("Etap 2");
         };
};

class Etap_3 : public Etap {
public:
        virtual void wypisz_nr_etapu(){
                lcd.setCursor(10,0);
                lcd.print("Etap 3");   
        };
};

         
void Wykonaj_Etapy(){
     Etap *etap_nr[2];
     etap_nr[0] = new Etap_1;
      delete etap_nr[0];
     etap_nr[1] = new Etap_2;
      delete etap_nr[1];
     etap_nr[2] = new Etap_3;  
      delete etap_nr[2];      
    };
  
void setup() {
  
      Serial.begin(9600);
      lcd.begin(16,2);
      czujnik.begin();
      
      pinMode(buzzer_port,OUTPUT);
      digitalWrite(buzzer_port,LOW);
      
      pinMode(ssr_port,OUTPUT);
      digitalWrite(ssr_port,HIGH);    

      Wykonaj_Etapy();  
     
};

void loop() {
  
 
};
/* Program został napisany przez Welflon */
