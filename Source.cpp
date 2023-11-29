#include <windows.h>
#include <fstream>
#include "resource.h"
#include <stdlib.h>
#include <commctrl.h> 
#include <vector>
#include <algorithm>
#include <ctime>
#include <string>
#include <cstdio>

struct nodo_doctor {
	char cedula[10];
	char namedoc[30];
	char user[20];
	char password[12];
	char foto[MAX_PATH];
	nodo_doctor* ant;
	nodo_doctor* sig;
};

struct nodo_clients {
	char date[20];  // La fecha debe ser una cadena con formato "DD/MM/AAAA"
	char hour[10];  // La hora debe ser una cadena con formato "HH:MM"  
	char nameclient[30];
	char phone[15];
	char pettype[15];
	char namepet[15];
	char motive[50];
	char total[10];
	char status[15];
	nodo_clients* ant;
	nodo_clients* sig;
};

nodo_doctor* lista_doctores = nullptr;
nodo_clients* lista_clientes = nullptr;
nodo_clients* lista_auxiliar;
nodo_clients* modificar;

nodo_doctor* aux = 0, * prim = 0, * ult = 0;
nodo_clients* auxiliar = 0, * primero = 0, * ultimo = 0;

//Ventanas
LRESULT CALLBACK AltaCitas(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK EliminarCita(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK ModificarCita(HWND, UINT, WPARAM, LPARAM); 
LRESULT CALLBACK ModificarAltaCita(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK FiltrarCita(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK VenInfoDocModif(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK VenAgenda(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK VenInfoDoc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK VenMenu(HWND, UINT, WPARAM, LPARAM);//Ventana menu inicio
LRESULT CALLBACK VenInicioSesion(HWND, UINT, WPARAM, LPARAM);

//Prototipo extras funciones 
void agregarcliente(nodo_clients* nuevoCliente);
void eliminarcliente(char NomElim[50]);
void busquedaNom(char NomModif[50]);
void busquedaFechas(char fechaInicio[11], char fechaFin[11]);
void agregardoc(nodo_doctor* aux);
bool compararFechas(tm fecha1, tm fecha2);

//Prototipos leer y escribir archivo 
void leer_info_doctor(const char* archivo);
void escribir_info_doctor(const char* archivo);
void leer_info_citas(const char* archivo);
void escribir_info_citas(const char* archivo);

//Prototipos tiempo
tm obtenerFechaHoraActual();
tm convertirSystemTimeATm(const SYSTEMTIME& st);
time_t convertirFechaATime(const char* fecha);

//Leer archivo datos del doctor
void leer_info_doctor(const char* archivo) {
	std::ifstream arch_doctor;
	arch_doctor.open(archivo, std::ios::binary);
	if (arch_doctor.is_open()) {
		nodo_doctor* nuevo_doctor = new nodo_doctor;
		arch_doctor.read(reinterpret_cast<char*>(nuevo_doctor), sizeof(nodo_doctor));
		while (!arch_doctor.eof()) {
			agregardoc(nuevo_doctor);

			nuevo_doctor = new nodo_doctor;
			arch_doctor.read(reinterpret_cast<char*>(nuevo_doctor), sizeof(nodo_doctor));
		}
		arch_doctor.close();
	}
	else {
	}
}

//Escribir archivo datos del doctor
void escribir_info_doctor(const char* archivo) {
	std::ofstream arch_doctor;
	arch_doctor.open(archivo, std::ios::trunc | std::ios::binary);
	if (arch_doctor.is_open()) {
		// Recorrer toda la lista ligada, nodo por nodo
		nodo_doctor* aux = lista_doctores;

		while (aux != nullptr) {
			// Guardar en el archivo cada nodo
			arch_doctor.write(reinterpret_cast<char*>(aux), sizeof(nodo_doctor));

			// Leer el siguiente nodo
			aux = aux->sig;
		}
		arch_doctor.close();
	}
	else {
	}
}

void agregardoc(nodo_doctor* aux) {
	aux->ant = 0;
	aux->sig = 0;
	if (lista_doctores == 0) {
		lista_doctores = aux;
	}
	else
	{
		ult->sig = aux;
		aux->ant = ult;
	}
	ult = aux;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrev, PSTR cmdLine, int cShow)
{
	//Al inicio lee los archivos de la información del doctor y las citas 
	leer_info_doctor("arch_doctores.dat");
	leer_info_citas("arch_citas.dat");
	HWND hInicioSesion = CreateDialog(hInst, MAKEINTRESOURCE(INICIARSESION), NULL, VenInicioSesion);
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	ShowWindow(hInicioSesion, cShow);

	while (GetMessage(&msg, NULL, NULL, NULL))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	// Antes de salir, guarda la información del doctor
	escribir_info_doctor("arch_doctores.dat");

	return 0;
}

//VENTANA INICIARSESION-------------------------------------------------------------------------------------------------------------------------
const char usuario_predefinido[] = "Veterinario";
const char contrasena_predefinida[] = "progra123*";
LRESULT CALLBACK VenInicioSesion(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_CLOSE:
	{
		int respuesta = MessageBox(hwnd, "¿Deseas cerrar el programa?", "Confirmacion", MB_YESNO | MB_ICONWARNING);
		if (respuesta == IDYES)
		{
			DestroyWindow(hwnd);
		}

	}break;

	case WM_DESTROY:
	{
		PostQuitMessage(0);
	}break;

	case WM_COMMAND:
	{
		if (LOWORD(wParam) == BTN_INGRESAR_ISESION && HIWORD(wParam) == BN_CLICKED) {
			HWND hUsuario = GetDlgItem(hwnd, BTN_USUARIO_ISESION);
			int usuariolenght = GetWindowTextLength(hUsuario);
			char buffer[256];
			GetWindowText(hUsuario, buffer, sizeof(buffer));

			HWND hContrasena = GetDlgItem(hwnd, BTN_CONTRA_ISESION);
			int contralenght = GetWindowTextLength(hContrasena);
			char buffer1[256];
			GetWindowText(hContrasena, buffer1, sizeof(buffer1));

			if (usuariolenght < 4) {
				MessageBox(hwnd, "El nombre de usuario debe tener mínimo 4 caracteres", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
			}

			if (contralenght != 10 || strcmp(buffer1, contrasena_predefinida) != 0) {
				MessageBox(hwnd, "La contraseña debe ser 'progra123*'", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
			}

			if (4 <= usuariolenght && contralenght == 10 && strcmp(buffer, usuario_predefinido) == 0) {
				// Usuario y contraseña correctos
				EndDialog(hwnd, 0);
				HWND hMenu = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(MENU_INICIO), hwnd, VenMenu);
				ShowWindow(hMenu, SW_SHOW);
			}
			else {
				MessageBox(hwnd, "Usuario o contraseña incorrectos", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
			}
		}
	}
	}
	return FALSE;
}

//VENTANA MENU_INICIO-------------------------------------------------------------------------------------------------------------------------

LRESULT CALLBACK VenMenu(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_INFODOC_INICIO && HIWORD(wParam) == BN_CLICKED) //Botón información del doctor 
		{
			EndDialog(hwnd, 0);
			HWND hInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(INFO_DOC), hwnd, VenInfoDoc);
			ShowWindow(hInfoDoctor, SW_SHOW); //Cierra el menú y muestra la ventana de información del doctor
		}
		if (LOWORD(wParam) == BTN_MANEJOC_INICIO && HIWORD(wParam) == BN_CLICKED) // Botón manejo de citas 
		{
			EndDialog(hwnd, 0);
			HWND hCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(A_CITA), hwnd, AltaCitas);
			ShowWindow(hCitas, SW_SHOW); //Cierra el menú y muestra la ventana para dar de alta citas
		}
		if (LOWORD(wParam) == BTN_AGENDA_INICIO && HIWORD(wParam) == BN_CLICKED) //Botón agenda 
		{
			EndDialog(hwnd, 0);
			HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
			ShowWindow(hAgenda, SW_SHOW); //Cierra el menú y muestra la ventana agenda
		}
		if (LOWORD(wParam) == BTN_SALIR_INICIO && HIWORD(wParam) == BN_CLICKED) //Botón salir
		{
			int respuesta = MessageBox(hwnd, "El programa esta por cerrarse, ¿deseas continuar?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
		}
	}
	}
	return FALSE;
}

//VENTANA INFO_DOC-------------------------------------------------------------------------------------------------------------------------

void ActualizarInfoDoc(HWND hwnd, nodo_doctor* infoActualizada) {
	HWND hNomDoc = GetDlgItem(hwnd, BTN_NOMBRE_ID);
	HWND hCedulaProf = GetDlgItem(hwnd, BTN_CEDULA_ID);
	HWND hUsu = GetDlgItem(hwnd, BTN_CLAVE_ID);
	HWND hContra = GetDlgItem(hwnd, BTN_CONTRA_ID);

	SetWindowText(hNomDoc, infoActualizada->namedoc);
	SetWindowText(hCedulaProf, infoActualizada->cedula);
	SetWindowText(hUsu, infoActualizada->user);
	SetWindowText(hContra, infoActualizada->password);
}

LRESULT CALLBACK VenInfoDoc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_INITDIALOG: {
		aux = lista_doctores;
		if (aux != nullptr) {
			// Hay al menos un doctor registrado
			HWND hNomDoc = GetDlgItem(hwnd, BTN_NOMBRE_ID);
			HWND hCedulaProf = GetDlgItem(hwnd, BTN_CEDULA_ID);
			HWND hUsu = GetDlgItem(hwnd, BTN_CLAVE_ID);
			HWND hContra = GetDlgItem(hwnd, BTN_CONTRA_ID);

			SetWindowText(hNomDoc, aux->namedoc);
			SetWindowText(hCedulaProf, aux->cedula);
			SetWindowText(hUsu, aux->user);
			SetWindowText(hContra, aux->password);
		}
		else {
		}
	} break;

	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_MANEJOC_ID && HIWORD(wParam) == BN_CLICKED) // Botón manejo de citas
		{
			EndDialog(hwnd, 0);
			HWND hCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(A_CITA), hwnd, AltaCitas);
			ShowWindow(hCitas, SW_SHOW); //Cierra la ventana y abre manejo de citas
		}
		if (LOWORD(wParam) == BTN_AGENDA_ID && HIWORD(wParam) == BN_CLICKED) //Botón agenda
		{
			EndDialog(hwnd, 0);
			HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
			ShowWindow(hAgenda, SW_SHOW); //Cierra la ventana y abre la agenda 
		}
		if (LOWORD(wParam) == BTN_SALIR_ID && HIWORD(wParam) == BN_CLICKED) //Botón salir 
		{
			int respuesta = MessageBox(hwnd, "El programa esta por cerrarse, ¿deseas continuar?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
		}
		if (LOWORD(wParam) == BTN_EDITINFO_ID && HIWORD(wParam) == BN_CLICKED) //Botón editar info del doctor
		{
			EndDialog(hwnd, 0);
			HWND hInfoDocModif = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(INFO_DOC_EDIT), hwnd, VenInfoDocModif);
			ShowWindow(hInfoDocModif, SW_SHOW); //Cierra la ventana y abre para editar la info del doctor
		}
	}break;

	}
	return FALSE;
}

///VENTANA INFO_DOC_EDIT-------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK VenInfoDocModif(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_INITDIALOG: {
		nodo_doctor* inicio = aux;
		aux = lista_doctores;
		if (aux == NULL) {
			// No hay doctor registrado, permitir ingresar la información
		}
		else {
			HWND hNomDocModif = GetDlgItem(hwnd, BTN_NOMBRE_IDE);
			HWND hCedulaProfModif = GetDlgItem(hwnd, BTN_CEDULA_IDE);
			HWND hUsuModif = GetDlgItem(hwnd, BTN_CLAVE_IDE);
			HWND hContraModif = GetDlgItem(hwnd, BTN_CONTRA_IDE);

			while (aux != nullptr) {
				SetWindowText(hNomDocModif, aux->namedoc);
				SetWindowText(hCedulaProfModif, aux->cedula);
				SetWindowText(hUsuModif, aux->user);
				SetWindowText(hContraModif, aux->password);
				aux = aux->sig;
			}
			aux = inicio;
		}
	} break;


	case WM_COMMAND: {

		if (LOWORD(wParam) == BTN_MANEJOC_IDE && HIWORD(wParam) == BN_CLICKED) // Botón manejo de citas 
		{
			EndDialog(hwnd, 0);
			HWND hCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(A_CITA), hwnd, AltaCitas);
			ShowWindow(hCitas, SW_SHOW);
		}
		if (LOWORD(wParam) == BTN_AGENDA_IDE && HIWORD(wParam) == BN_CLICKED) //Botón agenda 
		{
			EndDialog(hwnd, 0);
			HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
			ShowWindow(hAgenda, SW_SHOW);
		}
		if (LOWORD(wParam) == BTN_SALIR_IDE && HIWORD(wParam) == BN_CLICKED) //Botón salir  
		{
			int respuesta = MessageBox(hwnd, "El programa esta por cerrarse, ¿deseas continuar?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
		}
		if (LOWORD(wParam) == BTN_GUARDAR_IDE && HIWORD(wParam) == BN_CLICKED)
		{
			HWND hNomDoc = GetDlgItem(hwnd, BTN_NOMBRE_IDE);
			char NombreDoctor[30];
			GetWindowText(hNomDoc, NombreDoctor, sizeof(NombreDoctor));

			// Validación: el nombre del doctor no puede contener números
			bool contieneNumeros = false;
			for (int i = 0; NombreDoctor[i] != '\0'; ++i) {
				if (isdigit(NombreDoctor[i])) {
					contieneNumeros = true;
					break;
				}
			}

			if (contieneNumeros) {
				MessageBox(hwnd, "El nombre del doctor no puede contener números", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				return FALSE; // No continuar con el proceso de guardado
			}

			HWND hCedula = GetDlgItem(hwnd, BTN_CEDULA_IDE);
			char cedprof[10];
			GetWindowText(hCedula, cedprof, sizeof(cedprof));

			HWND hUsuarioMod = GetDlgItem(hwnd, BTN_CLAVE_IDE);
			char usumod[20];
			GetWindowText(hUsuarioMod, usumod, sizeof(usumod));

			HWND hContraMod = GetDlgItem(hwnd, BTN_CONTRA_IDE);
			char contramod[20];
			GetWindowText(hContraMod, contramod, sizeof(contramod));

			if (aux == nullptr) {
				// No hay doctor registrado, permitir ingresar la información y agregarla a la lista
				nodo_doctor* docmodif = new nodo_doctor();
				strcpy_s(docmodif->namedoc, NombreDoctor);
				strcpy_s(docmodif->cedula, cedprof);
				strcpy_s(docmodif->user, usumod);
				strcpy_s(docmodif->password, contramod);

				ActualizarInfoDoc(GetParent(hwnd), docmodif);

				agregardoc(docmodif);

				// Guardar la información actualizada en el archivo
				escribir_info_doctor("arch_doctores.dat");

				MessageBox(hwnd, "Informacion guardada con exito", "", MB_OK);
			}
			else {
				// Elimina el nodo existente y crea uno nuevo con la información modificada
				delete aux;
				aux = new nodo_doctor();
				strcpy_s(aux->namedoc, NombreDoctor);
				strcpy_s(aux->cedula, cedprof);
				strcpy_s(aux->user, usumod);
				strcpy_s(aux->password, contramod);

				// Guardar la información actualizada en el archivo
				escribir_info_doctor("arch_doctores.dat");

				MessageBox(hwnd, "Información guardada con éxito", "", MB_OK);
				EndDialog(hwnd, 0);
				HWND hMenu = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(MENU_INICIO), hwnd, VenMenu);
				ShowWindow(hMenu, SW_SHOW);
			}
		}
	} break;
	}
	return FALSE;
}

///VENTANA AGENDA-------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK VenAgenda(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) { 
	nodo_clients* busqueda = 0;
	switch (msg)
	{
	case WM_INITDIALOG: {
		int idx = 0;
		HWND hListaCitas = GetDlgItem(hwnd, LISTA_CITAS_AGENDA);
		auxiliar = lista_clientes;
		if (auxiliar == NULL)
		{
			SendMessage(hListaCitas, LB_ADDSTRING, 0, (LPARAM)"NO HAY NINGUNA CITA AGENDADA");
		}
		else {
			while (auxiliar != 0) {
				// Convierte la fecha a time_t
				time_t fechaCita = convertirFechaATime(auxiliar->date);

				// Obtener la fecha y hora actuales
				time_t fechaHoraActual = time(0);

				if (fechaCita != -1 && fechaCita >= fechaHoraActual) {
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Fecha:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->date);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Hora:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->hour);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Cliente:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->nameclient);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Teléfono:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->phone);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Especie de la mascota:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->pettype);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Mascota:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->namepet);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Motivo:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->motive);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Total:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->total);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"Estatus:");
					SendDlgItemMessage(hwnd, LISTA_CITAS_AGENDA, LB_ADDSTRING, idx, (LPARAM)auxiliar->status);
					SendMessage(hListaCitas, LB_ADDSTRING, idx, (LPARAM)"\n");
					idx++;
				}
				auxiliar = auxiliar->sig;
			}
		}
	}break;
	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_INFODOC_AGENDA && HIWORD(wParam) == BN_CLICKED) 
		{
			EndDialog(hwnd, 0);
			HWND hInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(INFO_DOC), hwnd, VenInfoDoc);
			ShowWindow(hInfoDoctor, SW_SHOW); 
		}
		if (LOWORD(wParam) == BTN_MANEJOC_AGENDA && HIWORD(wParam) == BN_CLICKED) 
		{
			EndDialog(hwnd, 0);
			HWND hCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(A_CITA), hwnd, AltaCitas);
			ShowWindow(hCitas, SW_SHOW); 
		}
		if (LOWORD(wParam) == BTN_ELIMINARC_AGENDA && HIWORD(wParam) == BN_CLICKED)
		{
			EndDialog(hwnd, 0);
			HWND hEliminarCita = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(ELIMINARCITA), hwnd, EliminarCita);
			ShowWindow(hEliminarCita, SW_SHOW);
		}
		if (LOWORD(wParam) == BTN_MODIFICARC_AGENDA && HIWORD(wParam) == BN_CLICKED)
		{
			EndDialog(hwnd, 0);
			HWND hModificarCita = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(PRE_MODIFCITA), hwnd, ModificarCita);
			ShowWindow(hModificarCita, SW_SHOW);
		}
		if (LOWORD(wParam) == BTN_FILTRADO_AGENDA && HIWORD(wParam) == BN_CLICKED)
		{
			EndDialog(hwnd, 0);
			HWND hFiltrado = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(FILTRADO), hwnd, FiltrarCita);
			ShowWindow(hFiltrado, SW_SHOW);
		}
		if (LOWORD(wParam) == BTN_SALIR_AGENDA && HIWORD(wParam) == BN_CLICKED) 
		{
			int respuesta = MessageBox(hwnd, "El programa esta por cerrarse, ¿deseas continuar?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
		}
	}break;
	}
	return FALSE;
}

///VENTANA A_CITAS-------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK AltaCitas(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_INITDIALOG: {
		char perro[] = "Perro";
		char gato[] = "Gato";
		char ave[] = "Ave";
		char otra[] = "Otra";

		char estatusP[] = "Pendiente";
		char estatusE[] = "Efectuada";
		char estatusC[] = "Cancelada";

		HWND hListaMascotas = GetDlgItem(hwnd, COMBO_ESPECIE_ACITA); //Opciones tipo de mascota
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)perro);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)gato);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)ave);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)otra);

		HWND hEstatus = GetDlgItem(hwnd, BTN_ESTATUS_ACITA); //Opciones estatus 
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusP);
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusE);
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusC);
	}break;
	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_INFODOC_ACITA && HIWORD(wParam) == BN_CLICKED) 
		{
			EndDialog(hwnd, 0);
			HWND hInfoDoctor = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(INFO_DOC), hwnd, VenInfoDoc);
			ShowWindow(hInfoDoctor, SW_SHOW); 
		}
		if (LOWORD(wParam) == BTN_AGENDA_ACITA && HIWORD(wParam) == BN_CLICKED) 
		{
			EndDialog(hwnd, 0);
			HWND hCitas = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda); 
			ShowWindow(hCitas, SW_SHOW); 
		}
		if (LOWORD(wParam) == BTN_GUARDAR_ACITA && HIWORD(wParam) == BN_CLICKED)  
		{
			bool validardatos = true;
			bool validarnom = true;
			bool validarcelular = true;
			bool validarnombremascota = true;
			bool validarcosto = true;

			//Obtener la fecha
			HWND hFecha = GetDlgItem(hwnd, IDC_FECHA_ACITA);
			SYSTEMTIME stFecha;
			ZeroMemory(&stFecha, sizeof(stFecha));
			SendMessage(hFecha, DTM_GETSYSTEMTIME, 0, (LPARAM)&stFecha);

			// Obtener la hora
			HWND hHora = GetDlgItem(hwnd, IDC_HORA_ACITA);
			SYSTEMTIME stHora;
			ZeroMemory(&stHora, sizeof(stHora));
			SendMessage(hHora, DTM_GETSYSTEMTIME, 0, (LPARAM)&stHora);

			char FechaCita[100];
			sprintf_s(FechaCita, "%02d/%02d/%d", stFecha.wDay, stFecha.wMonth, stFecha.wYear);

			char HoraCita[100];
			sprintf_s(HoraCita, "%02d:%02d", stHora.wHour, stHora.wMinute);

			// Obtener la fecha y hora actual
			tm fechaHoraActual = obtenerFechaHoraActual();

			// Comparar la fecha seleccionada con la fecha actual
			if (compararFechas(fechaHoraActual, convertirSystemTimeATm(stFecha))) {
				MessageBox(hwnd, "No se puede agendar una cita con fecha anterior a la actual", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hNombreCliente = GetDlgItem(hwnd, BTN_NOMBRE_ACITA);
			char nomcliente[50];
			GetWindowText(hNombreCliente, nomcliente, sizeof(nomcliente));
			int nomlong = GetWindowTextLength(hNombreCliente);
			if (nomlong == 0) { //Si no contiene nada el editcontrol da un avisp
				MessageBox(hwnd, "Debe registrar un nombre", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < nomlong; i++) {
				if (isalpha(nomcliente[i])); //Busca letras caracter por caracter
				{
					validarnom = true;
					validardatos = true;
				}
				if (isdigit(nomcliente[i])) { //Busca numeros en el nombre
					validarnom = false;
					validardatos = false; //Si encuentra un número el bool será falso y no dejará guardar la información 					
				}
			}
			if (validarnom == false) {
				MessageBox(hwnd, "El nombre del cliente solo acepta letras", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hTelefono = GetDlgItem(hwnd, BTN_TELEFONO_ACITA);
			char telefono[15];
			GetWindowText(hTelefono, telefono, sizeof(telefono));
			int digitos = GetWindowTextLength(hTelefono); //Obtener la cantidad de dígitos del celular

			if (digitos != 10)
			{
				MessageBox(hwnd, "El telefono debe tener 10 digitos", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < digitos; i++) {
				if (isalpha(telefono[i])); //Busca letras caracter por caracter
				{
					validarcelular = false; //Si encuentra una letra el bool será falso y no dejará guardar la información 
					validardatos = false;
				}
				if (isdigit(telefono[i])) { //Valida que haya números
					validarcelular = true;
					validardatos = true;
				}
			}
			if (validarcelular == false) {
				MessageBox(hwnd, "El telefono solo acepta numeros", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hEspecieMascota = GetDlgItem(hwnd, COMBO_ESPECIE_ACITA);
			char TipoMascota[30];
			GetWindowText(hEspecieMascota, TipoMascota, sizeof(TipoMascota));
			int EspecieMascotaLong = SendMessage(hEspecieMascota, CB_GETCURSEL, 0, 0); //Obtener el texto del combobox
			if (EspecieMascotaLong == CB_ERR) { //Validar que si el combobox no tiene nada arroje un messagebox
				MessageBox(hwnd, "Seleccione el tipo de mascota", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hNombreMascota = GetDlgItem(hwnd, BTN_MASCOTA_ACITA);
			char nommascota[50];
			GetWindowText(hNombreMascota, nommascota, sizeof(nommascota));
			int mascotalong = GetWindowTextLength(hNombreMascota);
			if (mascotalong == 0) { //Si no contiene nada el editcontrol da un aviso 
				MessageBox(hwnd, "Debe registrar el nombre de la mascota", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < mascotalong; i++) {
				if (isalpha(nommascota[i])); //Busca letras carcater por caracter
				{
					validarnombremascota = true;
					validardatos = true;
				}
				if (isdigit(nommascota[i])) { //Busca números en el nombre
					validarnombremascota = false;
					validardatos = false; //Si encuentra un número el bool será falso y no dejará guardar la información 

				}
			}
			if (validarnombremascota == false) {
				MessageBox(hwnd, "El nombre de la mascota solo acepta letras", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hConsulta = GetDlgItem(hwnd, BTN_MOTIVO_ACITA);
			char consulta[100];
			GetWindowText(hConsulta, consulta, sizeof(consulta));
			int consultalong = GetWindowTextLength(hConsulta);
			if (consultalong == 0)
			{
				MessageBox(hwnd, "Ingrese el motivo de la consulta", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hCosto = GetDlgItem(hwnd, BTN_COSTO_ACITA);
			char costo[20];
			GetWindowText(hCosto, costo, sizeof(costo));
			int costolong = GetWindowTextLength(hCosto);
			if (costolong == 0)
			{
				MessageBox(hwnd, "Ingrese un precio", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < costolong; i++) {
				if (isalpha(costo[i])); //Busca letras caracter por caracter
				{
					validarcosto = false; //Si encuentra una letra el bool será falso y no dejará guardar la información
					validardatos = false;
				}
				if (isdigit(costo[i])) { //Valida que haya números 
					validarcosto = true;
					validardatos = true;
				}
			}
			if (validarcosto == false) {
				MessageBox(hwnd, "El costo solo acepta numeros", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hEstatusConsu = GetDlgItem(hwnd, BTN_ESTATUS_ACITA);
			char ConsultaEst[30];
			GetWindowText(hEstatusConsu, ConsultaEst, sizeof(ConsultaEst));
			int EstatusConsLong = SendMessage(hEstatusConsu, CB_GETCURSEL, 0, 0); //Obtener el texto del combobox
			if (EstatusConsLong == CB_ERR) { //Validar que si el combobox no tiene nada arroje un messagebox
				MessageBox(hwnd, "Seleccione el estatus de la consulta", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			if (validardatos == true) { //Valida si los datos ingresados son correctos
				nodo_clients* cliente = new nodo_clients();
				strcpy_s(cliente->date, FechaCita);
				strcpy_s(cliente->hour, HoraCita);
				strcpy_s(cliente->nameclient, nomcliente);
				strcpy_s(cliente->phone, telefono);
				strcpy_s(cliente->pettype, TipoMascota);
				strcpy_s(cliente->namepet, nommascota);
				strcpy_s(cliente->motive, consulta);
				strcpy_s(cliente->total, costo);
				strcpy_s(cliente->status, ConsultaEst);
				agregarcliente(cliente);

				escribir_info_citas("arch_citas.dat");
				MessageBox(hwnd, "Informacion guardada con exito", "", MB_OK);
				SendMessage(hFecha, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hHora, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hNombreCliente, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hTelefono, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hEspecieMascota, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hNombreMascota, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hConsulta, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hCosto, WM_SETTEXT, 0, (LPARAM)"");
				SendMessage(hEstatusConsu, WM_SETTEXT, 0, (LPARAM)"");
				EndDialog(hwnd, 0);

				HWND hMenu = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(MENU_INICIO), hwnd, VenMenu);
				ShowWindow(hMenu, SW_SHOW);
			}
		}
		if (LOWORD(wParam) == BTN_SALIR_ACITA && HIWORD(wParam) == BN_CLICKED) 
		{
			int respuesta = MessageBox(hwnd, "El programa esta por cerrarse, ¿deseas continuar?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				DestroyWindow(hwnd);
				PostQuitMessage(0);
			}
		}
	}break;
	}
	return FALSE;
}

bool compararFechas(tm fecha1, tm fecha2) {
	if (fecha1.tm_year != fecha2.tm_year) {
		return fecha1.tm_year > fecha2.tm_year;
	}
	if (fecha1.tm_mon != fecha2.tm_mon) {
		return fecha1.tm_mon > fecha2.tm_mon;
	}
	if (fecha1.tm_mday != fecha2.tm_mday) {
		return fecha1.tm_mday > fecha2.tm_mday;
	}
	if (fecha1.tm_hour != fecha2.tm_hour) {
		return fecha1.tm_hour > fecha2.tm_hour;
	}
	return fecha1.tm_min > fecha2.tm_min;
}

// Función para obtener la fecha y hora actual
tm obtenerFechaHoraActual() {
	time_t tiempoActual;
	time(&tiempoActual);

	tm resultado;
	localtime_s(&resultado, &tiempoActual);

	return resultado;
}

tm convertirSystemTimeATm(const SYSTEMTIME& st) {
	tm resultado;
	resultado.tm_sec = st.wSecond;
	resultado.tm_min = st.wMinute;
	resultado.tm_hour = st.wHour;
	resultado.tm_mday = st.wDay;
	resultado.tm_mon = st.wMonth - 1;  // tm_mon es de 0 a 11
	resultado.tm_year = st.wYear - 1900;  // tm_year es el año desde 1900
	resultado.tm_isdst = -1;  // -1 indica que el horario de verano es desconocido
	return resultado;
}

time_t convertirFechaATime(const char* fecha) {
	// Asumiendo que la fecha tiene el formato "DD/MM/YYYY"
	int day, month, year;
	if (sscanf_s(fecha, "%d/%d/%d", &day, &month, &year) == 3) {
		tm tmFecha = {};
		tmFecha.tm_mday = day;
		tmFecha.tm_mon = month - 1; // Ajustar el mes
		tmFecha.tm_year = year - 1900; // Ajustar el año

		return std::mktime(&tmFecha);
	}

	return -1;
}

void agregarcliente(nodo_clients* auxiliar) {
	auxiliar->ant = nullptr;
	auxiliar->sig = nullptr;

	if (lista_clientes == nullptr) {
		// La lista está vacía,  asigna el nuevo cliente como el primero
		lista_clientes = auxiliar;
		ultimo = auxiliar;
	}
	else {
		// Buscar la posición correcta para insertar según la fecha y hora
		nodo_clients* actual = lista_clientes;
		while (actual != nullptr) {
			tm fechaHoraActual, fechaHoraAuxiliar;

			// Convertir cadenas de fecha y hora a estructuras tm
			sscanf_s(actual->date, "%d/%d/%d", &fechaHoraActual.tm_mday, &fechaHoraActual.tm_mon, &fechaHoraActual.tm_year);
			sscanf_s(actual->hour, "%d:%d", &fechaHoraActual.tm_hour, &fechaHoraActual.tm_min);

			sscanf_s(auxiliar->date, "%d/%d/%d", &fechaHoraAuxiliar.tm_mday, &fechaHoraAuxiliar.tm_mon, &fechaHoraAuxiliar.tm_year);
			sscanf_s(auxiliar->hour, "%d:%d", &fechaHoraAuxiliar.tm_hour, &fechaHoraAuxiliar.tm_min);

			// Comparar las fechas y horas utilizando la función de comparación personalizada
			if (compararFechas(fechaHoraActual, fechaHoraAuxiliar)) {
				break;
			}

			actual = actual->sig;
		}

		if (actual == nullptr) {
			// El nuevo cliente tiene la fecha y hora más grandes, al final
			ultimo->sig = auxiliar;
			auxiliar->ant = ultimo;
			ultimo = auxiliar;
		}
		else {
			// Insertar el nuevo cliente antes del cliente actual
			auxiliar->sig = actual;
			auxiliar->ant = actual->ant;
			if (actual->ant != nullptr) {
				actual->ant->sig = auxiliar;
			}
			else {
				// El nuevo cliente será el primero de la lista
				lista_clientes = auxiliar;
			}
			actual->ant = auxiliar;
		}
	}
}

//Escribir archivo de citas
void escribir_info_citas(const char* archivo) {
	std::ofstream arch_citas;
	arch_citas.open(archivo, std::ios::trunc | std::ios::binary);
	if (arch_citas.is_open()) {
		nodo_clients* aux = lista_clientes; 

		while (aux != nullptr) {
			// Guardar en el archivo cada nodo
			arch_citas.write(reinterpret_cast<char*>(aux), sizeof(nodo_clients));

			// Leer el siguiente nodo
			aux = aux->sig;
		}
		arch_citas.close();
	}
	else {
		MessageBox(nullptr, "Error al abrir el archivo para escribir.", "Error", MB_OK | MB_ICONERROR);
	}
}

//Leer archivo de citas
void leer_info_citas(const char* archivo) {
	std::ifstream arch_citas;
	arch_citas.open(archivo, std::ios::binary);
	if (arch_citas.is_open()) {
		nodo_clients* nuevo_cliente = new nodo_clients;
		arch_citas.read(reinterpret_cast<char*>(nuevo_cliente), sizeof(nodo_clients));
		while (!arch_citas.eof()) {
			nodo_clients* aux = new nodo_clients;
			aux = nuevo_cliente;

			agregarcliente(aux);

			nuevo_cliente = new nodo_clients;
			arch_citas.read(reinterpret_cast<char*>(nuevo_cliente), sizeof(nodo_clients));
		}
		arch_citas.close();
	}
	else {
		MessageBox(nullptr, "Error al abrir el archivo para leer.", "Error", MB_OK | MB_ICONERROR);
	}
}

///VENTANA ELIMINARCITA-------------------------------------------------------------------------------------------------------------------------
LRESULT CALLBACK EliminarCita(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_DELETE && HIWORD(wParam) == BN_CLICKED)
		{
			HWND hElimCita = GetDlgItem(hwnd, IDC_ELIMCITA);
			char NomElim[50];
			GetWindowText(hElimCita, NomElim, sizeof(NomElim));
			int ElimLong = GetWindowTextLength(hElimCita);

			if (ElimLong == 0) {
				MessageBox(hwnd, "Debes introducir un nombre", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
			}
			else
			{

				eliminarcliente(NomElim);
				escribir_info_citas("arch_citas.dat");

				EndDialog(hwnd, 0);
				HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
				ShowWindow(hAgenda, SW_SHOW);

			}
		}
		if (LOWORD(wParam) == BTN_CANCEL && HIWORD(wParam) == BN_CLICKED)
		{
			int respuesta = MessageBox(hwnd, "¿Deseas cancelar la operacion?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				EndDialog(hwnd, 0);
				HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
				ShowWindow(hAgenda, SW_SHOW); //Si se cancela la operación, vuelve a la ventana de cancelar
			}

		}
	}break;
	}
	return FALSE;
}

void eliminarcliente(char NomElim[50]) {
	nodo_clients* auxi1 = lista_clientes;
	nodo_clients* anterior = NULL;

	if (auxi1 == NULL) {
		MessageBox(0, "La lista está vacía", "ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	while (auxi1 != NULL) {
		if (strcmp(auxi1->nameclient, NomElim) == 0) {

			if (anterior == NULL) {

				lista_clientes = auxi1->sig;
				if (lista_clientes != NULL) {
					lista_clientes->ant = NULL;
				}
			}
			else {
				anterior->sig = auxi1->sig;
				if (auxi1->sig != NULL) {
					auxi1->sig->ant = anterior;
				}
				if (auxi1 == lista_clientes) {
					lista_clientes = anterior;
				}
			}

			delete auxi1;
			MessageBox(0, "El nodo ha sido eliminado exitosamente", "", MB_OK);
			return;
		}
		anterior = auxi1;
		auxi1 = auxi1->sig;
	}

}

///VENTANA PRE_MODIFCITA-------------------------------------------------------------------------------------------------------------------------
//Buscar el nombre de la persona de la cita
LRESULT CALLBACK ModificarCita(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_COMMAND: {

		if (LOWORD(wParam) == BTN_MODIFCITA && HIWORD(wParam) == BN_CLICKED) {
			HWND hNomModif = GetDlgItem(hwnd, NOMC_MODIFCITA);
			char NomModif[50];
			GetWindowText(hNomModif, NomModif, sizeof(NomModif));
			int ModifLong = GetWindowTextLength(hNomModif);
			bool encontrado = false;

			if (ModifLong == 0) {
				MessageBox(hwnd, "Debes introducir un nombre", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}
			else {
				auxiliar = lista_clientes;
				if (auxiliar == NULL) {
					MessageBox(hwnd, "La agenda esta vacia", "ERROR", MB_OK | MB_ICONERROR);
				}
				while (auxiliar != NULL && encontrado != true)
				{
					if (strcmp(auxiliar->nameclient, NomModif) == 0)
					{
						encontrado = true;
					}
					auxiliar = auxiliar->sig;
				}
				if (encontrado) {
					busquedaNom(NomModif);
					EndDialog(hwnd, 0);
					HWND hModCita = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(MODIFCITA), hwnd, ModificarAltaCita);
					ShowWindow(hModCita, SW_SHOW);
				}
				else
				{
					MessageBox(hwnd, "No se encontro la cita", "ERROR", MB_OK | MB_ICONERROR);
				}

			}

		}
		if (LOWORD(wParam) == BTN_CANCEL_MC && HIWORD(wParam) == BN_CLICKED) {
			int respuesta = MessageBox(hwnd, "¿Deseas cancelar la operacion?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES)
			{
				EndDialog(hwnd, 0);
				HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
				ShowWindow(hAgenda, SW_SHOW); //SI SE CANCELA LA OPERACION, VUELVE A LA VENTANA DE CANCELAR
			}
		}
	}break;
	}
	return FALSE;
}

///VENTANA MODIFCITA-------------------------------------------------------------------------------------------------------------------------
//Modificar los datos
LRESULT CALLBACK ModificarAltaCita(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg)
	{
	case WM_INITDIALOG: {
		char perro[] = "Perro";
		char gato[] = "Gato";
		char ave[] = "Ave";
		char otra[] = "Otra";

		char estatusP[] = "Pendiente";
		char estatusE[] = "Efectuada";
		char estatusC[] = "Cancelada";

		HWND hListaMascotas = GetDlgItem(hwnd, ESPECIE_MODIFICABLE); //Tipo de mascota 
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)perro);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)gato);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)ave);
		SendMessage(hListaMascotas, CB_ADDSTRING, NULL, (LPARAM)otra);

		HWND hEstatus = GetDlgItem(hwnd, ESTATUS_MODIFICABLE); //Opciones para estatus 
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusP);
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusE);
		SendMessage(hEstatus, CB_ADDSTRING, NULL, (LPARAM)estatusC);

		auxiliar = lista_auxiliar;
		HWND hFechaModif = GetDlgItem(hwnd, FECHA_MODIFICABL);
		HWND hHoraModif = GetDlgItem(hwnd, HORA_MODIFICABLE);
		HWND hNomClienteModif = GetDlgItem(hwnd, CLIENTE_MODIFICABLE);
		HWND hTelefonoModif = GetDlgItem(hwnd, TELEFONO_MODIFICABLE);
		HWND hEspecieModif = GetDlgItem(hwnd, ESPECIE_MODIFICABLE);
		HWND hNomMascotaModif = GetDlgItem(hwnd, MASCOTA_MODIFICABLE);
		HWND hMotivoModif = GetDlgItem(hwnd, MOTIVO_MODIFICABLE);
		HWND hCostoModif = GetDlgItem(hwnd, COSTO_MODIFICABLE);
		HWND hEstatusModif = GetDlgItem(hwnd, ESTATUS_MODIFICABLE);

		while (auxiliar != NULL) {
			SetWindowText(hNomClienteModif, auxiliar->nameclient);
			SetWindowText(hTelefonoModif, auxiliar->phone);
			SetWindowText(hEspecieModif, auxiliar->pettype);
			SetWindowText(hNomMascotaModif, auxiliar->namepet);
			SetWindowText(hMotivoModif, auxiliar->motive);
			SetWindowText(hCostoModif, auxiliar->total);
			SetWindowText(hEstatusModif, auxiliar->status);
			auxiliar = auxiliar->sig;
		}
	}break;
	case WM_COMMAND: {
		if (LOWORD(wParam) == GUARDAR_MODIFICACION && HIWORD(wParam) == BN_CLICKED) {

			bool validardatos = true;
			bool validarnom = true;
			bool validarcelular = true;
			bool validarnombremascota = true;
			bool validarcosto = true;

			//Obtener la fecha
			HWND hFecha = GetDlgItem(hwnd, FECHA_MODIFICABL); 
			SYSTEMTIME stFecha;
			ZeroMemory(&stFecha, sizeof(stFecha));
			SendMessage(hFecha, DTM_GETSYSTEMTIME, 0, (LPARAM)&stFecha);

			// Obtener la hora
			HWND hHora = GetDlgItem(hwnd, HORA_MODIFICABLE);  
			SYSTEMTIME stHora;
			ZeroMemory(&stHora, sizeof(stHora));
			SendMessage(hHora, DTM_GETSYSTEMTIME, 0, (LPARAM)&stHora);

			char FechaCita[100];
			sprintf_s(FechaCita, "%02d/%02d/%d", stFecha.wDay, stFecha.wMonth, stFecha.wYear);

			char HoraCita[100];
			sprintf_s(HoraCita, "%02d:%02d", stHora.wHour, stHora.wMinute);

			// Obtener la fecha y hora actual
			tm fechaHoraActual = obtenerFechaHoraActual();

			// Comparar la fecha seleccionada con la fecha actual
			if (compararFechas(fechaHoraActual, convertirSystemTimeATm(stFecha))) {
				MessageBox(hwnd, "No se puede agendar una cita con fecha anterior a la actual", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hNombreCliente = GetDlgItem(hwnd, CLIENTE_MODIFICABLE); 
			char nomcliente[50];
			GetWindowText(hNombreCliente, nomcliente, sizeof(nomcliente));
			int nomlong = GetWindowTextLength(hNombreCliente);
			if (nomlong == 0) { //Si no contiene nada el edit control da un aviso
				MessageBox(hwnd, "Debe registrar un nombre", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < nomlong; i++) {
				if (isalpha(nomcliente[i])); //Busca letras caracter por caracter
				{
					validarnom = true;
					validardatos = true;
				}
				if (isdigit(nomcliente[i])) { //Busca números en el nombre
					validarnom = false;
					validardatos = false; //Si encuentra un númeor en el bool será falso y no deja guardar la información					
				}
			}
			if (validarnom == false) {
				MessageBox(hwnd, "El nombre del cliente solo acepta letras", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hTelefono = GetDlgItem(hwnd, TELEFONO_MODIFICABLE);
			char telefono[15];
			GetWindowText(hTelefono, telefono, sizeof(telefono));
			int digitos = GetWindowTextLength(hTelefono); //Obtener la cantidad de digitos del celular

			if (digitos != 10)
			{
				MessageBox(hwnd, "El telefono debe tener 10 digitos", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < digitos; i++) {
				if (isalpha(telefono[i])); //Busca letras caracter por caracter
				{
					validarcelular = false; //Si encuentra una letra el bool será falso y no dejará guardar la información 
					validardatos = false;
				}
				if (isdigit(telefono[i])) { //Valida que haya números
					validarcelular = true;
					validardatos = true;
				}
			}
			if (validarcelular == false) {
				MessageBox(hwnd, "El telefono solo acepta numeros", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hEspecieMascota = GetDlgItem(hwnd, ESPECIE_MODIFICABLE);
			char TipoMascota[30];
			GetWindowText(hEspecieMascota, TipoMascota, sizeof(TipoMascota));
			int EspecieMascotaLong = SendMessage(hEspecieMascota, CB_GETCURSEL, 0, 0); //Obtener el texto del combobox
			if (EspecieMascotaLong == CB_ERR) { //Validar que si el combobox no tiene nada arroje un messagebox
				MessageBox(hwnd, "Seleccione el tipo de mascota", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hNombreMascota = GetDlgItem(hwnd, MASCOTA_MODIFICABLE);
			char nommascota[50];
			GetWindowText(hNombreMascota, nommascota, sizeof(nommascota));
			int mascotalong = GetWindowTextLength(hNombreMascota);
			if (mascotalong == 0) { //Si no contiene nada el editcontrol da un aviso 
				MessageBox(hwnd, "Debe registrar el nombre de la mascota", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < mascotalong; i++) {
				if (isalpha(nommascota[i])); //Busca letras caracter por caracter
				{
					validarnombremascota = true;
					validardatos = true;
				}
				if (isdigit(nommascota[i])) { //Busca nunmeros en el nombre 
					validarnombremascota = false;
					validardatos = false; //Si encuentra un número el bool será falso y no dejará guardar la información 

				}
			}
			if (validarnombremascota == false) {
				MessageBox(hwnd, "El nombre de la mascota solo acepta letras", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hConsulta = GetDlgItem(hwnd, MOTIVO_MODIFICABLE);
			char consulta[100];
			GetWindowText(hConsulta, consulta, sizeof(consulta));
			int consultalong = GetWindowTextLength(hConsulta);
			if (consultalong == 0)
			{
				MessageBox(hwnd, "Ingrese el motivo de la consulta", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			HWND hCosto = GetDlgItem(hwnd, COSTO_MODIFICABLE);
			char costo[20];
			GetWindowText(hCosto, costo, sizeof(costo));
			int costolong = GetWindowTextLength(hCosto);
			if (costolong == 0)
			{
				MessageBox(hwnd, "Ingrese un precio", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}
			for (int i = 0; i < costolong; i++) {
				if (isalpha(costo[i])); //Busca letras carcater por caracter
				{
					validarcosto = false; //Si encuentra una letra el bool será falso y no dejará guardar la información 
					validardatos = false;
				}
				if (isdigit(costo[i])) { //Valida que haya números
					validarcosto = true;
					validardatos = true;
				}
			}
			if (validarcosto == false) {
				MessageBox(hwnd, "El costo solo acepta numeros", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				break;
			}

			HWND hEstatusConsu = GetDlgItem(hwnd, ESTATUS_MODIFICABLE);
			char ConsultaEst[30];
			GetWindowText(hEstatusConsu, ConsultaEst, sizeof(ConsultaEst));
			int EstatusConsLong = SendMessage(hEstatusConsu, CB_GETCURSEL, 0, 0); //Obtener el texto del combobox 
			if (EstatusConsLong == CB_ERR) { //Validar que si el combobox no tiene nada arroje un messagebox 
				MessageBox(hwnd, "Seleccione el estatus de la consulta", "ADVERTENCIA", MB_OK | MB_ICONWARNING);
				validardatos = false;
				break;
			}

			if (validardatos == true) {
				modificar = lista_clientes;
				auxiliar = lista_auxiliar;

				while (modificar != NULL)
				{
					if (strcmp(modificar->nameclient, auxiliar->nameclient) == 0) {
						strcpy_s(auxiliar->date, FechaCita);
						strcpy_s(auxiliar->hour, HoraCita);
						strcpy_s(auxiliar->nameclient, nomcliente);
						strcpy_s(auxiliar->phone, telefono);
						strcpy_s(auxiliar->pettype, TipoMascota);
						strcpy_s(auxiliar->namepet, nommascota);
						strcpy_s(auxiliar->motive, consulta);
						strcpy_s(auxiliar->total, costo);
						strcpy_s(auxiliar->status, ConsultaEst);

						strcpy_s(modificar->date, auxiliar->date);
						strcpy_s(modificar->hour, auxiliar->hour);
						strcpy_s(modificar->nameclient, auxiliar->nameclient);
						strcpy_s(modificar->phone, auxiliar->phone);
						strcpy_s(modificar->pettype, auxiliar->pettype);
						strcpy_s(modificar->namepet, auxiliar->namepet);
						strcpy_s(modificar->motive, auxiliar->motive);
						strcpy_s(modificar->total, auxiliar->total);
						strcpy_s(modificar->status, auxiliar->status);
					}
					modificar = modificar->sig;
				}
				escribir_info_citas("arch_citas.dat");

				MessageBox(hwnd, "Informacion guardada con exito", "", MB_OK);
				EndDialog(hwnd, 0);
				HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
				ShowWindow(hAgenda, SW_SHOW);
			}
		}

	}break;
	}
	return FALSE;
}

//VOID PARA BUSCAR LA CITA
void busquedaNom(char NomModif[50]) {
	nodo_clients* auxi1 = lista_clientes;
	lista_auxiliar = new nodo_clients();
	bool encontrado = false;

	if (auxi1 == NULL) {
		MessageBox(0, "La lista está vacía", "ERROR", MB_OK | MB_ICONERROR);
	}
	if (auxi1 != NULL) {

		while (auxi1 != NULL && encontrado != true)
		{
			if (strcmp(auxi1->nameclient, NomModif) == 0)
			{
				strcpy_s(lista_auxiliar->date, auxi1->date);
				strcpy_s(lista_auxiliar->nameclient, auxi1->nameclient);
				strcpy_s(lista_auxiliar->phone, auxi1->phone);
				strcpy_s(lista_auxiliar->pettype, auxi1->pettype);
				strcpy_s(lista_auxiliar->namepet, auxi1->namepet);
				strcpy_s(lista_auxiliar->motive, auxi1->motive);
				strcpy_s(lista_auxiliar->total, auxi1->total);
				strcpy_s(lista_auxiliar->status, auxi1->status);
				encontrado = true;
			}
			auxi1 = auxi1->sig;
		}
	}

}

///VENTANA FILTRADO-------------------------------------------------------------------------------------------------------------------------
void convertirFecha(const char* fecha, struct tm& tmFecha) {
	sscanf_s(fecha, "%d/%d/%d", &tmFecha.tm_mday, &tmFecha.tm_mon, &tmFecha.tm_year);
	tmFecha.tm_mon -= 1;  // Ajustar el mes, ya que en tm_mon enero es 0
	tmFecha.tm_year -= 1900;  // Ajustar el año, ya que en tm_year se cuenta desde 1900
}

void busquedaFechas(char fechaInicio[11], char fechaFin[11]) {
	nodo_clients* auxi1 = lista_clientes;
	lista_auxiliar = new nodo_clients();
	bool encontrado = false;

	if (auxi1 == NULL) {
		MessageBox(0, "La lista está vacía", "ERROR", MB_OK | MB_ICONERROR);
		return;
	}

	// Convertir las fechas de cadena a estructuras de tiempo
	struct tm tmFechaInicio = { 0 };
	struct tm tmFechaFin = { 0 };
	convertirFecha(fechaInicio, tmFechaInicio);
	convertirFecha(fechaFin, tmFechaFin);

	while (auxi1 != NULL) {
		// Convertir la fecha del nodo actual a una estructura de tiempo
		struct tm tmFechaAuxi1 = { 0 };
		convertirFecha(auxi1->date, tmFechaAuxi1);

		// Comparar las fechas
		if (difftime(mktime(&tmFechaAuxi1), mktime(&tmFechaInicio)) >= 0 &&
			difftime(mktime(&tmFechaAuxi1), mktime(&tmFechaFin)) <= 0) {
			// Copiar la información relevante a lista_auxiliar
			strcpy_s(lista_auxiliar->date, auxi1->date);
			strcpy_s(lista_auxiliar->nameclient, auxi1->nameclient);
			strcpy_s(lista_auxiliar->phone, auxi1->phone);
			strcpy_s(lista_auxiliar->pettype, auxi1->pettype);
			strcpy_s(lista_auxiliar->namepet, auxi1->namepet);
			strcpy_s(lista_auxiliar->motive, auxi1->motive);
			strcpy_s(lista_auxiliar->total, auxi1->total);
			strcpy_s(lista_auxiliar->status, auxi1->status);
			encontrado = true;
			break;  // Romper el bucle si se encuentra una coincidencia
		}
		auxi1 = auxi1->sig;
	}

	if (!encontrado) {
		MessageBox(0, "No se encontraron citas en el rango de fechas", "INFORMACIÓN", MB_OK | MB_ICONINFORMATION);
	}
}

LRESULT CALLBACK FiltrarCita(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
	case WM_COMMAND: {
		if (LOWORD(wParam) == BTN_FILTRAR && HIWORD(wParam) == BN_CLICKED) {
			// Mostrar los resultados en el ListBox
			HWND hListBox = GetDlgItem(hwnd, LISTA_CITAS_FILTRADO);

			// Limpiar el contenido existente en el ListBox
			SendMessage(hListBox, LB_RESETCONTENT, 0, 0);

			// Obtener la fecha de inicio
			SYSTEMTIME stFechaInicio;
			HWND hDateTimePicker1 = GetDlgItem(hwnd, FECHA_FILTRO1);
			SendMessage(hDateTimePicker1, DTM_GETSYSTEMTIME, 0, (LPARAM)&stFechaInicio);

			// Obtener la fecha de fin
			SYSTEMTIME stFechaFin;
			HWND hDateTimePicker2 = GetDlgItem(hwnd, FECHA_FILTRO2);
			SendMessage(hDateTimePicker2, DTM_GETSYSTEMTIME, 0, (LPARAM)&stFechaFin);

			// Convertir las fechas a cadenas en el formato deseado 
			char fechaInicio[11];
			char fechaFin[11];
			GetDateFormat(LOCALE_USER_DEFAULT, 0, &stFechaInicio, "dd/MM/yyyy", fechaInicio, sizeof(fechaInicio));
			GetDateFormat(LOCALE_USER_DEFAULT, 0, &stFechaFin, "dd/MM/yyyy", fechaFin, sizeof(fechaFin));

			// Llamar a la función para buscar por fechas
			busquedaFechas(fechaInicio, fechaFin);

			// Reinicializar idx
			int idx = 0;

			// Agregar elementos al ListBox
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Fecha:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->date);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Cliente:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->nameclient);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Teléfono:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->phone);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Especie de la mascota:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->pettype);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Mascota:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->namepet);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Motivo:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->motive);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Total:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->total);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"Estatus:");
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)lista_auxiliar->status);
			SendMessage(hListBox, LB_ADDSTRING, idx, (LPARAM)"\n");
		}

		if (LOWORD(wParam) == BTN_CANCELAR && HIWORD(wParam) == BN_CLICKED) {
			int respuesta = MessageBox(hwnd, "¿Deseas cancelar la operación?", "ADVERTENCIA", MB_YESNO | MB_ICONWARNING);
			if (respuesta == IDYES) {
				EndDialog(hwnd, 0);
				HWND hAgenda = CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(AGENDA), hwnd, VenAgenda);
				ShowWindow(hAgenda, SW_SHOW); // Si se cancela, vuelve a la ventana de agenda 
			}
		}
	} break;
	}
	return FALSE;
}
