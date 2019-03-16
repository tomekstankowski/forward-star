#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Małe grafy proste (o co najwyżej 16 wierzchołkach) reprezentowane przy pomocy macierzy sąsiedztwa.
class Graph:

    # Wyjątek pojawiający się przy próbie konwersji z błędnej reprezentacji tekstowej.
    class G6Error( Exception ): pass

    # Wyjątek pojawiający się przy próbie usunięcia z grafu ostatniego wierzchołka.
    class NoVerticesError( Exception ): pass

    # Wyjątek pojawiający się przy próbie dodania 17-tego wierzchołka.
    class TooManyVerticesError( Exception ): pass

    # Tworzy graf o podanej reprezentacji tekstowej (domyślnie: 1-wierzchołkowy graf pusty).
    def __init__( self, text = "@" ):
        self.fromString( text )

    # Zwraca liczbę wierzchołków grafu.
    def order( self ):
        return self.__order

    # Dodaje do grafu nowy izolowany wierzchołek.
    def addVertex( self ):
        if self.__order == 16:
            raise Graph.TooManyVerticesError( "too many vertices" )
        for v in range( self.__order ):
            self.__matrix[v] += [False]
        self.__order += 1
        self.__matrix += [ [False] * self.__order ]

    # Usuwa z grafu wskazany wierzchołek.
    def deleteVertex( self, u ):
        if self.__order == 1:
            raise Graph.NoVerticesError( "graph must have vertices" )
        del self.__matrix[u]
        for v in range( self.__order - 1 ):
            del self.__matrix[v][u]
        self.__order -= 1

    # Zwraca informację o tym, czy podane wierzchołki sąsiadują z sobą.
    def isEdge( self, u, v ):
        return self.__matrix[u][v]

    # Dodaje podaną krawędź.
    def addEdge( self, u, v ):
        self.__matrix[u][v] = self.__matrix[v][u] = True

    # Usuwa podaną krawędź.
    def deleteEdge( self, u, v ):
        self.__matrix[u][v] = self.__matrix[v][u] = False

    # Przekształca reprezentację tekstową grafu w graf.
    def fromString( self, text ):
        try:
            t, k = iter( text ), 0
            c = ord( next( t ) ) - 63
            if c < 1 or c > 16:
                raise Graph.G6Error( "wrong order: {0}".format( c ) )
            self.__order = c
            self.__matrix = [[False for v in range( c )] for u in range( c )]
            for v in range( 1, self.__order ):
                for u in range( v ):
                    if k == 0:
                        c = ord( next( t ) ) - 63
                        if c < 0 or c > 63:
                            raise Graph.G6Error( "wrong character: {0}".format( c + 63 ) )
                        k = 6
                    k -= 1
                    if (c & (1 << k)) != 0:
                        self.__matrix[u][v] = self.__matrix[v][u] = True
        except StopIteration:
            raise Graph.G6Error( "too short text" )
        try:
            c = ord( next( t ) )
            raise Graph.G6Error( "too long text" )
        except StopIteration:
            pass

    # Przekształca graf w reprezentację tekstową.
    def __str__( self ):
        text, k, c = chr( self.__order + 63 ), 5, 0
        for v in range( 1,self.__order ):
            for u in range( v ):
                if self.__matrix[u][v]:
                    c |= (1 << k)
                if k == 0:
                    text += chr( c + 63 )
                    k, c = 6, 0
                k -= 1
        if k != 5:
            text += chr( c + 63 )
        return text

    # Test równości dwóch reprezentacji grafów.
    def __eq__( self, other ):
        if self.__order != other.order():
            return False
        for v in range( 1, self.__order ):
            for u in range( v ):
                if self.__matrix[u][v] != other.isEdge( u, v ):
                    return False
        return True

    # Test różności dwóch reprezentacji grafów.
    def __ne__( self, other ):
        if self.__order != other.order():
            return True
        for v in range( 1, self.__order ):
            for u in range( v ):
                if self.__matrix[u][v] != other.isEdge( u, v ):
                    return True
        return False
