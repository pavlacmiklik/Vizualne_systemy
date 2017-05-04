clear all;
clc;
%% Uprava vstupneho obrazku
% nacitanie
I = imread('C:\Users\Kaèenka\Desktop\hough\CD.bmp');
% obrazok do gray
img_gray = rgb2gray(I);
% gaussov filter
filter = fspecial('gaussian',3,3); 
img_gauss = imfilter(img_gray,filter);
% najdenie hran pomocou canny
Img = edge(img_gauss,'canny');
Img2 = edge(img_gray,'canny');
%% Houghova transformacia
r=3; % radius
r2 = r^2;
prah=4; %prahova hodnota ktoru odcitame od akumulatora 
Akumulator = zeros(size(Img)); % vytvoríme si akumulátor
[yNenulove xNenulove] = find(Img); % najdeme vsetky nenulove x,y (hrany)
nRiadkov = size(Img,1); % pocet riadkov v binary image
nStlpcov = size(Img,2); % pocet stlpcov v binary image

for n = 1:numel(xNenulove) % od 1 do poctu prvkov v xIndex 
    dolna=xNenulove(n)-r; %aktualny prvok xNenulove - radius
    horna=xNenulove(n)+r; %aktualny prvok xNenulove + radius
    if (dolna<1) %kazda dolna mensia ako 1 sa rovna 1 
        dolna=1; 
    end
    if (horna>nStlpcov) %kazda horna vacsia ako pocet stlpcov = poctu stlpcov
        horna=nStlpcov; 
    end
    for x0 = dolna:horna %prechadzame od dolnej po hornu
        yOdchylka = sqrt(r2-(xNenulove(n)-x0)^2); %pocitame odchylku
        y1 = round(yNenulove(n)-yOdchylka);
        y2 = round(yNenulove(n)+yOdchylka);
                
        if y1 < nRiadkov && y1 >= 1 %ak y01 je v rozmedzi 1 a poctu riadkov
            Akumulator(y1,x0) = Akumulator(y1,x0)+1; %zvys cislo v akumulatore na danej 
        end                              %pozicii o 1
        
        if y2 < nRiadkov && y2 >= 1 %ak y01 je v rozmedzi 1 a poctu riadkov
            Akumulator(y2,x0) = Akumulator(y2,x0)+1; %zvys cislo v akumulatore na danej
        end                              %pozicii o 1
    end
end
y = []; x = [];
AkumulatorMax = imregionalmax(Akumulator); %lokalne maximum
[mozne_y0 mozne_x0] = find(AkumulatorMax == 1); %zoberieme najdene lok. maxima, vyberieme z nich suradnice na ktorych je 1
AkumDocasny = Akumulator - prah; %od akumulatora odcitame prahovu hodnotu
for n = 1:numel(mozne_y0) %vykonavame od 1 po pocet prvkov v moznom y0
    if AkumDocasny(mozne_y0(n),mozne_x0(n)) >= 0 %ak docasny acc ma hodnotu viac ako 0 na aktualnych suradniciach mozne y0 a mozne x0
        y = [y;mozne_y0(n)]; %potom pridame do y, mozne y0 na aktualnych suradniciach
        x = [x;mozne_x0(n)]; %potom pridame do x, mozne x0 na aktualnych suradniciach
    end
end
%% Vykreslenie najdenych kruhov do povodneho obrazku
imshow(Img);
hold on;
nBodov = size(x);
nBodov = nBodov(1)
for i = 1:nBodov
       plot(x,y,'r.','MarkerSize',10) 
end
