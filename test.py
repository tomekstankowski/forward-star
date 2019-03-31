#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from graphs  import Graph
from sys     import argv
from time    import time
from zipfile import ZipFile

# test.py
#
# Tester struktur grafowych. Sposób użycia:
#
#   ./test.py KLASA MODUŁ
#
# KLASA powinna być klasą zdefiniowaną w module MODUŁ. Tester sprawdza, czy podana klasa prawidłowo reprezentuje grafy proste.
# Test polega na wykonaniu kilku elementarnych operacji na wszystkich parami nieizomorficznych grafach o co najwyżej 10
# wierzchołkach i porównaniu rezultatów z wzorcem. Testowi podlegają następujące operacje:
#   - konwersja z/do formatu g6;
#   - usuwanie i dodawanie wybranych wierzchołków;
#   - usuwanie, dodawanie i sprawdzanie, czy wybrane krawędzie istnieją.
# Tester kończy pracę w momencie stwierdzenia pierwszego błędu lub poprawnego zaliczenia całości.

try:
    exec( "from {0} import {1} as TestedGraph".format( argv[2], argv[1] ) )
    print( "Przygotowuję dane testowe. Proszę czekać." )
    with ZipFile( "graphs.zip" ) as data:
        graphs = "\n".join( [data.open( name ).read().decode() for name in data.namelist()] ).split()
    print( "Rozpoczynam testy klasy {0}:".format( argv[1] ) )
    print()

    start = time()
    # Test 1: Czy testowana klasa zawiera wszystkie wymagane pola i metody?
    print( "Test #1 -- wymagane pola i metody" )
    if [x for x in dir( TestedGraph ) if not x.startswith( "_" )] != [x for x in dir( Graph ) if not x.startswith( "_" )]:
        print( "!!! Klasa {0} nie zawiera wymaganych pól lub metod.".format( argv[2] ) )
        exit()
    print( "Test #1 zaliczony -- klasa {0} zawiera wszystkie wymagane pola i metody.".format( argv[2] ) )

    # Test 2: Czy wyjątki działają prawidłowo?
    print( "Test #2 -- wyjątki" )
    try:
        try: TestedGraph( "" )
        except TestedGraph.G6Error: pass
        else:
            print( "!!! Błędna obsługa konwersji zbyt krótkiego napisu." )
            exit()
        try: TestedGraph( "@ " )
        except TestedGraph.G6Error: pass
        else:
            print( "!!! Błędna obsługa konwersji zbyt długiego napisu." )
            exit()
        try: TestedGraph( "A*" )
        except TestedGraph.G6Error: pass
        else:
            print( "!!! Błędna obsługa konwersji błędnego napisu." )
            exit()
        try: TestedGraph().deleteVertex( 0 )
        except TestedGraph.NoVerticesError: pass
        else:
            print( "!!! Błędna obsługa usuwania wierzchołków." )
            exit()
        try: TestedGraph( "O????????????????????" ).addVertex()
        except TestedGraph.TooManyVerticesError: pass
        else:
            print( "!!! Błędna obsługa dodawania wierzchołków." )
            exit()
    except Exception as v:
        print( "!!! Pojawił się niespodziewany wyjątek {0}.".format( str( v ) ) )
        exit()
    print( "Test #2 zaliczony -- wyjątki działają prawidłowo." )

    n, p = 0, 0
    # Test 3: Które z operacji na grafach klasa wykonuje prawidłowo?
    print( "Test #3 -- operacje" )
    for g6 in graphs:
        n += 1
        if (n * 1000) // len( graphs ) > p:
            p += 1
            print( " {0}.{1}%".format( p // 10, p % 10 ) )

         # Test konwersji z/do formatu g6
        try:
            g, h = TestedGraph( g6 ), Graph( g6 )
            if str( g ) != g6 or h != g:
                print( "!!! Błędna konwersja z/do formatu g6 dla grafu {0}".format( g6 ) )
                exit()
        except Exception as e:
            print( "!!! Test konwersji z/do formatu g6 zakończył się dla grafu {0} wyjątkiem '{1}'".format( g6, str( e ) ) )
            exit()
        # Test dodawania i usuwania wierzchołków
        try:
            g, h = TestedGraph( g6 ), Graph( g6 )
            g.addVertex()
            g.deleteVertex( g.order() - 1 )
            if h != g:
                print( "!!! Błąd usuwania/dodawania wierzchołka dla grafu {0}".format( g6 ) )
                exit()
        except Exception as e:
            print( "!!! Test dodawania/usuwania wierzchołków zakończył się dla grafu {0} wyjątkiem '{1}'".format( g6, str( e ) ) )
            exit()
        # Test dodawania, usuwania i sprawdzania istnienia krawędzi
        try:
            g = TestedGraph( g6 )
            for u in range( g.order() ):
                v = (u - 2) % g.order()
                if u != v:
                    g.deleteEdge( u, v )
                    if g.isEdge( u,v ):
                        print( "!!! Błąd usuwania krawędzi dla grafu {0}".format( g6 ) )
                        exit()
                    g.addEdge( u, v )
                    if not g.isEdge( u,v ):
                        print(u,v)
                        print( "!!! Błąd dodawania krawędzi dla grafu {0}".format( g6 ) )
                        exit()
        except Exception as e:
            print( "!!! Test dodawania/usuwania krawędzi zakończył się dla grafu {0} wyjątkiem '{1}'".format( g6, str( e ) ) )
            exit()
    print( "Test #3 zaliczony -- operacje na grafach działają prawidłowo. Test trwał {0} sekund.".format( time() - start ) )
except ImportError:
    print( "Moduł {0} nie istnieje lub nie zawiera definicji klasy {1}.".format( argv[2], argv[1] ) )
except IndexError:
    print( "Sposób użycia: {0} KLASA MODUŁ".format( argv[0] ) )
