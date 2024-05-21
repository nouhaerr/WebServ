[NetworkingClient]
    └─> Recoit une reqest
        └─> [HttpRequestParser]
            └─> Analyse la reqest
                └─> Produit un obj HttpRequest
                    └─> [HttpRequest]
                        └─> Contient les informations de la reqest
                            └─> Utilise par le serveur pour traiter reqest
                                └─> [WebServer.cpp]
                                    └─> Utilise HttpRequest pour traiter la reqest
                                        └─> cree une HttpResponse
                                            └─> [HttpResponse]
                                                └─> Definit le code de statut et le corps
                                                    └─> Envoye via NetworkingClient
                                                        └─> [NetworkingClient]
                                                            └─> Envoie la reponse au client