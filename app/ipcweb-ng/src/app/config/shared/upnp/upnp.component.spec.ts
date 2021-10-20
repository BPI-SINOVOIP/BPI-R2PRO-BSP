import { async, ComponentFixture, TestBed } from '@angular/core/testing';

import { UpnpComponent } from './upnp.component';

describe('UpnpComponent', () => {
  let component: UpnpComponent;
  let fixture: ComponentFixture<UpnpComponent>;

  beforeEach(async(() => {
    TestBed.configureTestingModule({
      declarations: [ UpnpComponent ]
    })
    .compileComponents();
  }));

  beforeEach(() => {
    fixture = TestBed.createComponent(UpnpComponent);
    component = fixture.componentInstance;
    fixture.detectChanges();
  });

  it('should create', () => {
    expect(component).toBeTruthy();
  });
});
